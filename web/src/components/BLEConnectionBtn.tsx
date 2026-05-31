import { useConnectionStore } from '../stores/connection'
import { useTranslation } from 'react-i18next'

export default function BLEConnectionBtn() {
  const { t } = useTranslation()
  const { connected, connecting, connect, disconnect } = useConnectionStore()

  const handleClick = () => {
    if (connected) {
      disconnect()
    } else {
      if (!navigator.bluetooth) {
        alert('当前浏览器不支持 Web Bluetooth，请使用 Chrome/Edge 浏览器')
        return
      }
      connect()
    }
  }

  if (connecting) {
    return (
      <button className="inline-flex items-center gap-2 px-4 py-2 rounded-md bg-muted text-muted-foreground text-sm cursor-wait">
        <span className="w-2 h-2 rounded-full bg-yellow-400 animate-pulse" />
        {t('conn.connecting')}
      </button>
    )
  }

  if (connected) {
    return (
      <button
        onClick={handleClick}
        className="inline-flex items-center gap-2 px-4 py-2 rounded-md bg-green-50 text-green-700 hover:bg-green-100 text-sm"
      >
        <span className="w-2 h-2 rounded-full bg-green-500" />
        {t('conn.disconnect')}
      </button>
    )
  }

  return (
    <button
      onClick={handleClick}
      className="inline-flex items-center gap-2 px-4 py-2 rounded-md bg-primary text-primary-foreground hover:bg-primary/90 text-sm"
    >
      <span className="w-2 h-2 rounded-full bg-gray-400" />
      {t('conn.connect')}
    </button>
  )
}
