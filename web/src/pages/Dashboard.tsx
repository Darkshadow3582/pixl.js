import { useEffect, useState, useCallback } from 'react'
import { useTranslation } from 'react-i18next'
import { useConnectionStore } from '../stores/connection'
import { useDriveInfo } from '../hooks/useDriveInfo'
import DeviceInfoCard from '../components/DeviceInfoCard'
import StorageChart from '../components/StorageChart'
import Dialog from '../components/Dialog'
import * as proto from '../lib/pixl.proto'

export default function Dashboard() {
  const { t } = useTranslation()
  const { connected } = useConnectionStore()
  const { drives, fetchDrives } = useDriveInfo()
  const [confirmState, setConfirmState] = useState<{ msg: string; onOk: () => void } | null>(null)

  useEffect(() => {
    if (connected) {
      proto.get_version().then(res => {
        useConnectionStore.setState({
          version: res.data.ver,
          bleAddress: res.data.ble_addr,
        })
      })
      fetchDrives()
    }
  }, [connected, fetchDrives])

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

  if (!connected) {
    return (
      <div className="flex flex-col items-center justify-center h-full text-center">
        <div className="mb-6">
          <h1 className="text-3xl font-bold mb-2">Pixl.js</h1>
          <p className="text-muted-foreground">{t('dashboard.connect_title')}</p>
        </div>
        <button
          onClick={() => useConnectionStore.getState().connect()}
          className="inline-flex items-center gap-2 px-8 py-3 rounded-lg bg-primary text-primary-foreground hover:bg-primary/90 text-base font-medium"
        >
          {t('dashboard.connect_btn')}
        </button>
      </div>
    )
  }

  return (
    <div className="space-y-6">
      <h2 className="text-2xl font-bold">{t('dashboard.title')}</h2>
      <div className="grid grid-cols-1 md:grid-cols-3 gap-4">
        <DeviceInfoCard />
        {drives.map((drive, i) => (
          <StorageChart
            key={i}
            label={drive.label}
            used={drive.freeSize}
            total={drive.totalSize}
            status={drive.status}
          />
        ))}
      </div>
      <div className="rounded-lg border bg-card p-5">
        <h3 className="font-medium text-sm mb-3">{t('dashboard.quick_actions')}</h3>
        <div className="flex gap-3">
          <button
            onClick={handleEnterDFU}
            className="px-4 py-2 rounded-md bg-destructive text-destructive-foreground text-sm hover:bg-destructive/90"
          >
            {t('dashboard.dfu_btn')}
          </button>
          <button
            onClick={fetchDrives}
            className="px-4 py-2 rounded-md bg-secondary text-secondary-foreground text-sm hover:bg-secondary/80"
          >
            {t('dashboard.refresh_btn')}
          </button>
        </div>
      </div>

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
