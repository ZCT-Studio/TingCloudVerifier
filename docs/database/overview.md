# 数据库：三后端支持

TingCloudVerifier 同时支持 **SQLite**（默认）、**PostgreSQL**、**MySQL / MariaDB** 三种后端，运行时通过 `config.yaml database.type` 切换。**repo/service 层零改动**。

## 选择哪个？

| 后端 | 特点 | 适用场景 | vcpkg feature |
|---|---|---|---|
| **SQLite** | 零依赖、单文件、嵌入式、文件锁 | 单机部署、开发/测试、小流量 | 默认（总启用） |
| **PostgreSQL** | 生产级、并发好、MVCC、丰富 JSON | 生产、多节点、复杂查询 | `postgres` |
| **MySQL 8+ / MariaDB 11+** | 生态广、成熟稳定 | 已有 MySQL 运维栈 | `mysql` |

> 默认 triplet 编出来只有 SQLite。需要 PG/MySQL 时在 vcpkg manifest 安装 feature，一次编进去**运行时二选一**。

## 启用可选后端

```bash
# PostgreSQL
vcpkg install tingcloudverifier[postgres]

# MySQL
vcpkg install tingcloudverifier[mysql]

# 两个都装 (一次编进去, config.yaml 决定跑哪个)
vcpkg install tingcloudverifier[postgres,mysql]
```

CMake 自动检测：
- `find_package(PostgreSQL QUIET)` → 找不到 → 不启 `TCV_HAS_PGSQL` 宏
- `find_package(unofficial-libmysqlclient CONFIG QUIET)` → 找不到 → 不启 `TCV_HAS_MYSQL` 宏
- 运行时 `type=postgresql` 但编译没进 libpq → 抛 `"compiled without PostgreSQL support"`

## config.yaml

### SQLite（默认）

```yaml
database:
  type: "sqlite"
  sqlite_path: "./tcv.db"
```

### PostgreSQL

```yaml
database:
  type: "postgresql"
  postgresql:
    host: "127.0.0.1"
    port: 5432
    dbname: "tcv"
    user: "tcv"
    password: "xxx"          # 生产环境建议用 TCV_PG_PASSWORD env
    sslmode: "prefer"        # disable / prefer / require / verify-ca / verify-full
```

快速本地起一个：

```bash
docker run -d --name tcv-pg \
  -e POSTGRES_USER=tcv \
  -e POSTGRES_PASSWORD=tcv_test \
  -e POSTGRES_DB=tcv \
  -p 5432:5432 postgres:16
```

### MySQL

```yaml
database:
  type: "mysql"
  mysql:
    host: "127.0.0.1"
    port: 3306
    dbname: "tcv"
    user: "tcv"
    password: "xxx"
    connect_timeout_sec: 10
```

快速本地起一个：

```bash
docker run -d --name tcv-mysql \
  -e MYSQL_ROOT_PASSWORD=tcv_test \
  -e MYSQL_DATABASE=tcv \
  -e MYSQL_USER=tcv \
  -e MYSQL_PASSWORD=tcv_test \
  -p 3306:3306 mysql:8.4
```

## Migration 目录结构

```
migrations/
├── sqlite/      ← SQLite 方言
├── pgsql/       ← PostgreSQL 方言
├── mysql/       ← MySQL 方言
└── 001_initial.sql  ← legacy fallback (MigrationRunner 找不到 backend 目录时使用)
```

后端切换后 `MigrationRunner` 自动选对应子目录：
- SQLite → `migrations/sqlite/`
- PostgreSQL → `migrations/pgsql/`
- MySQL → `migrations/mysql/`

### 方言差异

SQLite 特有语法在三后端版本里分别改写：

| 语法点 | SQLite | PostgreSQL | MySQL |
|---|---|---|---|
| 自增主键 | `INTEGER PRIMARY KEY AUTOINCREMENT` | `BIGSERIAL PRIMARY KEY` | `BIGINT PRIMARY KEY AUTO_INCREMENT` |
| 幂等 INSERT | `INSERT OR IGNORE INTO` | `INSERT INTO ... ON CONFLICT (key) DO NOTHING` | `INSERT IGNORE INTO` |
| 当前时间戳 | `strftime('%s', 'now')` | `EXTRACT(EPOCH FROM NOW())::BIGINT` | `UNIX_TIMESTAMP()` |
| 事务 BEGIN | `BEGIN IMMEDIATE` (避免 busy) | `BEGIN` | `START TRANSACTION` |
| PRAGMA | `PRAGMA foreign_keys=ON;` | 不需要（默认开） | 不需要（默认开） |

## 内部架构

```
IDatabase (纯虚接口, idatabase.hpp)
  open / close / isOpen
  exec / execParams
  query / queryParams / queryScalar
  transaction / execSqlFile
  adaptParams(sql)   ← 占位符方言适配

SqliteDatabase     ← sqlite3 C API, 保留原始实现
PgsqlDatabase      ← libpq C API, adaptParams 把 ? 改成 $1 $2 ...
MysqlDatabase      ← libmysqlclient prepared statements

Database::instance() ← 门面类, 持有 unique_ptr<IDatabase>
  open(cfg)        按 cfg.type 创建具体实现
  execParams/queryParams/queryScalar
                   内部 adaptParams(sql) → packVec(args...) → impl_->execParams
  transaction      委托 impl_->transaction
```

Repo 层调用示例（**不变**）：

```cpp
// 任何后端下都一样
auto r = db.queryParams("SELECT * FROM apps WHERE appid = ?", appid);
db.execParams("INSERT INTO licenses(...) VALUES(?, ?, ?)", v1, v2, v3);
auto id = db.lastInsertId();
```

PgsqlDatabase 的 `adaptParams` 会把 `?` 占位符改成 `$1 $2 $3`，同时跳过字符串常量里的 `?`、注释里的 `?`——纯字符串替换，不碰业务 SQL。

## 业务主表（三后端共用）

```
apps              — 应用（APPID 全局唯一，dec_mode/dec_key/sign_enable/notice）
licenses          — 卡密（IP/device 绑定信息、解绑次数）
sessions          — 用户会话（admin/owner/subuser 统一表）
update_channels   — 版本通道（每个 APP 建 stable/rc/canary/alpha 四条）
nonces            — 防重放（UNIQUE appid + nonce）
ip_whitelist      — LOCAL 安全等级白名单
audit_logs        — 审计日志
schema_migrations — 版本追踪（MigrationRunner 自动建）
```

所有表的 `PRIMARY KEY` 都是整数自增，`TEXT` 字段三后端都支持，`FOREIGN KEY ... ON DELETE CASCADE` 三后端都支持。**没有用任何后端特有类型**（如 PostgreSQL 的 `JSONB`、MySQL 的 `DATETIME`）——全部用 `TEXT` + Unix timestamp 整数。

## CI 回归

GitHub Actions 里跑 2 个独立 job，只 smoke test migration + bootstrap：

```yaml
pg-test:     ubuntu-24.04 + postgres:16 docker + vcpkg[postgres] + /api/v1/auth/bootstrap-admin
mysql-test:  ubuntu-24.04 + mysql:8.4  docker + vcpkg[mysql]    + /api/v1/auth/bootstrap-admin
```

SQLite 是默认后端（9 平台 matrix），不跑额外 smoke test。

## 给新项目加第四种后端？

1. 写 `src/database/xxx_database.hpp` 继承 `IDatabase`
2. 实现 `open/close/exec/execParams/query/queryParams/queryScalar/transaction/begin/commit/rollback`
3. 如果有占位符方言（不是 `?`），override `adaptParams`
4. 在 `Database::open(cfg)` 里加一个 `else if (type == "xxx")` 分支
5. `MigrationRunner::backendDirName` 里加一个返回 `"xxx"` 的 case
6. 新建 `migrations/xxx/` 目录 + 方言 SQL
7. CMakeLists.txt 加 `find_package(XXX QUIET)` + `TCV_HAS_XXX=1` 编译宏
8. vcpkg.json 加 `features.xxx`
