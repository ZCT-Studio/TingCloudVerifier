-- Migration 002: add missing storage_mode to licenses
ALTER TABLE licenses ADD COLUMN storage_mode TEXT NOT NULL DEFAULT 'PLAIN';