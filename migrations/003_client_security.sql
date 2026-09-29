-- TingCloudVerifier migration 003: client security stack + app fields
-- Runs AFTER initial.sql (apps table already exists)

-- App 侧：签名开关 + 公告
ALTER TABLE apps ADD COLUMN sign_enable INTEGER NOT NULL DEFAULT 0;
ALTER TABLE apps ADD COLUMN notice     TEXT    NOT NULL DEFAULT '';

-- 注: dec_mode / dec_key 已在 initial.sql 预留, 不动表
-- 注: update_channels / update_versions 保留两张表, 不迁移
-- 注: licenses.binding_mode 保留但运行时以 app.binding_mode 为准
