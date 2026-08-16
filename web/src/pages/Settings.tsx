import { useState, useEffect, useCallback } from 'react'
import { useNavigate } from 'react-router-dom'
import { useTranslation } from 'react-i18next'
import { useConnectionStore } from '../stores/connection'
import Dialog from '../components/Dialog'
import * as proto from '../lib/pixl.proto'

export default function Settings() {
  const navigate = useNavigate()
  const { t, i18n } = useTranslation()
  const { connected, version } = useConnectionStore()
  const [confirmState, setConfirmState] = useState<{ msg: string; onOk: () => void } | null>(null)

  useEffect(() => {
    if (!connected) navigate('/')
  }, [connected, navigate])

  const confirm = useCallback((msg: string) => new Promise<boolean>(resolve => {
    setConfirmState({ msg, onOk: () => { setConfirmState(null); resolve(true) } })
  }), [])

  const handleEnterDFU = async () => {
    const ok = await confirm(t('dialog.dfu_confirm'))
    if (!ok) return
    await proto.enter_dfu()
    const ok2 = await confirm(t('dialog.dfu_update'))
    if (!ok2) return
    window.location.href = 'https://thegecko.github.io/web-bluetooth-dfu/examples/web.html'
  }

  return (
    <div className="max-w-xl space-y-6">
      <h2 className="text-2xl font-bold">{t('settings.title')}</h2>

      <section className="rounded-lg border bg-card p-5 space-y-4">
        <h3 className="font-medium">{t('settings.firmware')}</h3>
        <div className="flex items-center justify-between">
          <span className="text-sm text-muted-foreground">{t('settings.current_version', { version: version || '-' })}</span>
          <button
            onClick={handleEnterDFU}
            className="px-4 py-2 rounded-md bg-destructive text-destructive-foreground text-sm hover:bg-destructive/90"
          >
            {t('settings.dfu_btn')}
          </button>
        </div>
      </section>

      <section className="rounded-lg border bg-card p-5 space-y-4">
        <h3 className="font-medium">{t('settings.language')}</h3>
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
        <h3 className="font-medium">{t('settings.about')}</h3>
        <p className="text-sm text-muted-foreground">{t('settings.about_desc')}</p>
        <a
          href="https://github.com/solosky/pixl.js"
          target="_blank"
          rel="noopener noreferrer"
          className="text-sm text-primary hover:underline block"
        >
          GitHub
        </a>
        <p className="text-xs text-muted-foreground">{t('settings.license')}</p>
      </section>

      <Dialog
        open={!!confirmState}
        title={t('dialog.confirm')}
        onClose={() => setConfirmState(null)}
        footer={
          <>
            <button onClick={() => setConfirmState(null)} className="px-4 py-2 rounded-md border text-sm">{t('dialog.cancel')}</button>
            <button onClick={() => confirmState?.onOk()} className="px-4 py-2 rounded-md bg-primary text-primary-foreground text-sm">{t('dialog.ok')}</button>
          </>
        }
      >
        <p className="text-sm whitespace-pre-wrap">{confirmState?.msg}</p>
      </Dialog>
    </div>
  )
}
