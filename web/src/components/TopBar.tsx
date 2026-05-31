import { useTranslation } from 'react-i18next'
import BLEConnectionBtn from './BLEConnectionBtn'
import LanguageSwitcher from './LanguageSwitcher'
import { useConnectionStore } from '../stores/connection'

export default function TopBar() {
  const { t } = useTranslation()
  const { connected, version } = useConnectionStore()

  return (
    <header className="h-14 border-b bg-card flex items-center justify-between px-6">
      <div className="flex items-center gap-3">
        <span className="text-base font-bold">Pixl.js</span>
        {version && (
          <span className="text-xs text-muted-foreground bg-muted px-2 py-1 rounded">
            {t('device.version_prefix', { version })}
          </span>
        )}
      </div>
      <div className="flex items-center gap-3">
        {connected && <BLEConnectionBtn />}
        <LanguageSwitcher />
      </div>
    </header>
  )
}
