// @ts-ignore
import { defineConfig } from 'vitepress'

export default defineConfig({
  lang: 'zh-CN',
  title: 'TingCloudVerifier',
  description: '多平台软件授权验证服务端 · API 文档',

  lastUpdated: true,
  cleanUrls: true,

  themeConfig: {
    nav: [
      { text: 'API 概览', link: '/api/overview' },
      { text: '客户端安全', link: '/client/security' },
      { text: '数据库', link: '/database/overview' },
      { text: 'GitHub', link: 'https://github.com/<owner>/TingCloudVerifier' }
    ],

    sidebar: {
      '/database/': [
        { text: '数据库概览', link: '/database/overview' },
        { text: '启用可选后端', link: '/database/overview#启用可选后端' },
        { text: '配置 (SQLite / PG / MySQL)', link: '/database/overview#config-yaml' },
        { text: 'Migration 目录', link: '/database/overview#migration-目录结构' },
        { text: '方言差异对照表', link: '/database/overview#方言差异' },
        { text: '内部架构', link: '/database/overview#内部架构' },
        { text: '业务主表', link: '/database/overview#业务主表三后端共用' },
        { text: 'CI 回归', link: '/database/overview#ci-回归' },
        { text: '加第四种后端', link: '/database/overview#给新项目加第四种后端' }
      ],
      '/api/': [
        {
          text: '认证',
          items: [
            { text: 'bootstrap-admin', link: '/api/auth/bootstrap-admin' },
            { text: 'admin/login', link: '/api/auth/admin-login' },
            { text: 'owner/login', link: '/api/auth/owner-login' },
            { text: 'subuser/login', link: '/api/auth/subuser-login' },
            { text: 'logout', link: '/api/auth/logout' }
          ]
        },
        {
          text: 'Admin',
          items: [
            { text: 'owner/create', link: '/api/admin/owner-create' },
            { text: 'owner/balance', link: '/api/admin/owner-balance' },
            { text: 'audit/list', link: '/api/admin/audit-list' },
            { text: 'license-timer', link: '/api/admin/license-timer' }
          ]
        },
        {
          text: 'APP 管理',
          items: [
            { text: 'app/create', link: '/api/app/create' },
            { text: 'app/list', link: '/api/app/list' },
            { text: 'app/delete', link: '/api/app/delete' },
            { text: 'app/binding', link: '/api/app/binding' },
            { text: 'app/secret/regenerate', link: '/api/app/secret-regenerate' },
            { text: 'app/set-decode', link: '/api/app/set-decode' },
            { text: 'app/sign-enable', link: '/api/app/sign-enable' },
            { text: 'app/notice', link: '/api/app/notice' },
            { text: 'app/channel & version', link: '/api/app/channel-version' }
          ]
        },
        {
          text: '卡密管理',
          items: [
            { text: 'license/create', link: '/api/license/create' },
            { text: 'license/list', link: '/api/license/list' },
            { text: 'license 时间/类型操作', link: '/api/license/time-type' },
            { text: 'license 封禁/解绑/删除', link: '/api/license/ban-unbind' }
          ]
        },
        {
          text: '客户端',
          items: [
            { text: 'client/license/verify', link: '/api/client/license-verify' },
            { text: 'client/license/status', link: '/api/client/license-status' },
            { text: 'client/license/remaining', link: '/api/client/license-remaining' },
            { text: 'client/app/channels', link: '/api/client/app-channels' },
            { text: 'client/app/version', link: '/api/client/app-version' },
            { text: 'client/app/notice', link: '/api/client/app-notice' }
          ]
        }
      ],
      '/client/': [
        { text: '客户端安全层', link: '/client/security' },
        { text: '加密传输', link: '/client/encryption' },
        { text: '签名校验', link: '/client/signing' },
        { text: '完整示例', link: '/client/examples' }
      ]
    },

    socialLinks: [
      { icon: 'github', link: 'https://github.com/<owner>/TingCloudVerifier' }
    ],

    footer: {
      message: 'Apache 2.0 Licensed · © ZCT-Studio',
      copyright: 'TingCloudVerifier'
    },

    search: {
      provider: 'local'
    }
  }
})
