import { useTranslation } from 'react-i18next'
import { useConnectionStore } from '../stores/connection'
import * as proto from '../lib/pixl.proto'

export default function Settings() {
  const { i18n } = useTranslation()
  const { version } = useConnectionStore()

  const handleEnterDFU = async () => {
    if (confirm('是否进入DFU模式？')) {
      await proto.enter_dfu()
      if (confirm('设备已进入DFU模式，是否跳转到固件更新页面？')) {
        window.location.href = 'https://thegecko.github.io/web-bluetooth-dfu/examples/web.html'
      }
    }
  }

  return (
    <div className="max-w-xl space-y-6">
      <h2 className="text-2xl font-bold">设置</h2>

      <section className="rounded-lg border bg-card p-5 space-y-4">
        <h3 className="font-medium">固件</h3>
        <div className="flex items-center justify-between">
          <span className="text-sm text-muted-foreground">当前版本: {version || '-'}</span>
          <button
            onClick={handleEnterDFU}
            className="px-4 py-2 rounded-md bg-destructive text-destructive-foreground text-sm hover:bg-destructive/90"
          >
            进入 DFU 模式
          </button>
        </div>
      </section>

      <section className="rounded-lg border bg-card p-5 space-y-4">
        <h3 className="font-medium">语言</h3>
        <select
          value={i18n.language}
          onChange={(e) => i18n.changeLanguage(e.target.value)}
          className="h-9 rounded-md border border-input bg-background px-3 text-sm w-48"
        >
          <option value="zh-CN">简体中文</option>
          <option value="zh-TW">繁體中文</option>
          <option value="en">English</option>
          <option value="es">Español</option>
          <option value="ru">Русский</option>
          <option value="de">Deutsch</option>
          <option value="sv">Svenska</option>
          <option value="ja">日本語</option>
        </select>
      </section>

      <section className="rounded-lg border bg-card p-5 space-y-2">
        <h3 className="font-medium">关于</h3>
        <p className="text-sm text-muted-foreground">Pixl.js - Amiibo 模拟器</p>
        <a
          href="https://github.com/solosky/pixl.js"
          target="_blank"
          rel="noopener noreferrer"
          className="text-sm text-primary hover:underline block"
        >
          GitHub
        </a>
        <p className="text-xs text-muted-foreground">基于 GPL 2.0 开源协议发布</p>
      </section>
    </div>
  )
}
