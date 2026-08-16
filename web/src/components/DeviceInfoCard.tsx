import { useTranslation } from 'react-i18next'
import { useConnectionStore } from '../stores/connection'
import { Monitor } from 'lucide-react'

export default function DeviceInfoCard() {
  const { t } = useTranslation()
  const { connected, version, bleAddress } = useConnectionStore()

  return (
    <div className="rounded-lg border bg-card p-5">
      <div className="flex items-center gap-2 mb-4">
        <Monitor className="w-5 h-5 text-muted-foreground" />
        <h3 className="font-medium text-sm">{t('dashboard.device_info')}</h3>
      </div>
      <div className="space-y-3 text-sm">
        <div className="flex justify-between">
          <span className="text-muted-foreground">{t('dashboard.status')}</span>
          <span className={connected ? 'text-green-600' : 'text-muted-foreground'}>
            {connected ? t('dashboard.connected') : t('dashboard.disconnected')}
          </span>
        </div>
        <div className="flex justify-between">
          <span className="text-muted-foreground">{t('dashboard.firmware')}</span>
          <span>{version || '-'}</span>
        </div>
        <div className="flex justify-between">
          <span className="text-muted-foreground">{t('dashboard.ble_address')}</span>
          <span className="font-mono text-xs">{bleAddress || '-'}</span>
        </div>
      </div>
    </div>
  )
}
