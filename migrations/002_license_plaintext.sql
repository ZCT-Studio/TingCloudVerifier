-- Migration 002: 卡密改为明文存储，接口支持 license_id 或 license 双定位
-- SQLite 不支持 RENAME INDEX，所以：DROP + ADD

PRAGMA foreign_keys = OFF;

-- 重命名列
ALTER TABLE licenses RENAME COLUMN license_hash TO license;

-- 重建索引（名字保留，指向新列）
DROP INDEX IF EXISTS idx_licenses_license_hash;
CREATE INDEX IF NOT EXISTS idx_licenses_license ON licenses(license);

-- 如果之前没加，确保唯一索引（同一 APP 内卡密唯一）
CREATE UNIQUE INDEX IF NOT EXISTS idx_licenses_app_license ON licenses(app_id, license);

PRAGMA foreign_keys = ON;
