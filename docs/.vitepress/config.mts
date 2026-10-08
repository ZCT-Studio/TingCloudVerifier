import {defineConfig} from 'vitepress'

const apiItems = [
    {text: 'API 概览', link: '/api/overview'},
    {text: '客户端 · verify', link: '/api/client/license-verify'},
    {text: 'APP · set-decode', link: '/api/app/set-decode'},
    {text: 'APP · sign-enable', link: '/api/app/sign-enable'},
]

const clientItems = [
    {text: '客户端安全层', link: '/client/security'},
    {text: '加密传输', link: '/client/encryption'},
    {text: '签名校验', link: '/client/signing'},
    {text: '完整示例', link: '/client/examples'},
]

export default defineConfig({
    lang: 'zh-CN',
    title: 'TingCloudVerifier',
    description: '多平台软件授权验证服务端 · C++23 · Drogon · 三后端',
    
    lastUpdated: true,
    cleanUrls: true,
    
    head: [
        ['link', {rel: 'icon', href: '/favicon.svg', type: 'image/svg+xml'}],
        ['meta', {name: 'theme-color', content: '#6366f1'}],
        ['meta', {property: 'og:type', content: 'website'}],
        ['meta', {property: 'og:title', content: 'TingCloudVerifier — 多平台软件授权验证服务端'}],
        ['meta', {
            property: 'og:description',
            content: 'C++23 · Drogon · SQLite/PostgreSQL/MySQL · 卡密系统 · 签名防重放 · AES256-GCM'
        }],
        ['style', {}, ':root { --vp-c-brand-1: #6366f1; --vp-c-brand-2: #4f46e5; --vp-c-brand-3: #4338ca; --vp-c-brand-soft: rgba(99, 102, 241, 0.12); }'],
    ],
    
    themeConfig: {
        siteTitle: '🜛 TingCloudVerifier',
        
        nav: [
            {text: '文档', link: '/api/overview'},
            {text: '数据库', link: '/database/overview'},
            {text: 'GitHub', link: 'https://github.com/ZCT-Studio/TingCloudVerifier', target: '_blank'},
        ],
        
        sidebar: {
            '/api/': [
                {text: 'API 概览', items: apiItems},
            ],
            '/client/': [
                {text: '客户端', items: clientItems},
            ],
            '/database/': [
                {
                    text: '数据库', items: [
                        {text: '概览', link: '/database/overview'},
                        {text: 'SQLite / PG / MySQL 配置', link: '/database/overview#config-yaml'},
                        {text: 'Migration 目录结构', link: '/database/overview#migration-目录结构'},
                        {text: '三后端方言对照表', link: '/database/overview#方言差异'},
                    ]
                },
            ],
        },
        
        socialLinks: [
            {icon: 'github', link: 'https://github.com/ZCT-Studio/TingCloudVerifier', ariaLabel: 'GitHub'},
        ],
        
        footer: {
            message: 'Released under the Apache 2.0 License.',
            copyright: 'Copyright © ZCT-Studio',
        },
        
        search: {
            provider: 'local',
        },
        
        editLink: {
            pattern: 'https://github.com/ZCT-Studio/TingCloudVerifier/edit/main/docs/:path',
            text: '在 GitHub 上编辑此页',
        },
        
        outline: {
            level: [2, 3],
            label: '页面导航',
        },
        
        darkModeSwitchLabel: '主题',
        lightModeSwitchTitle: '切换到亮色模式',
        darkModeSwitchTitle: '切换到暗色模式',
        
        lastUpdated: {
            text: '最后更新于',
            formatOptions: {
                dateStyle: 'medium',
                timeStyle: 'short',
            },
        },
        
        docFooter: {
            prev: '上一页',
            next: '下一页',
        },
    },
})
