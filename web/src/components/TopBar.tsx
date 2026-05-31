import BLEConnectionBtn from './BLEConnectionBtn'
import LanguageSwitcher from './LanguageSwitcher'
import { useConnectionStore } from '../stores/connection'

export default function TopBar() {
  const { version } = useConnectionStore()

  return (
    <header className="h-14 border-b bg-card flex items-center justify-between px-6">
      <div className="flex items-center gap-3">
        {version && (
          <span className="text-xs text-muted-foreground bg-muted px-2 py-1 rounded">
            已连接 v{version}
          </span>
        )}
      </div>
      <div className="flex items-center gap-3">
        <BLEConnectionBtn />
        <LanguageSwitcher />
      </div>
    </header>
  )
}
