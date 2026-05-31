import { useConnectionStore } from '../stores/connection'
import { Monitor } from 'lucide-react'

export default function DeviceInfoCard() {
  const { connected, version, bleAddress } = useConnectionStore()

  return (
    <div className="rounded-lg border bg-card p-5">
      <div className="flex items-center gap-2 mb-4">
        <Monitor className="w-5 h-5 text-muted-foreground" />
        <h3 className="font-medium text-sm">设备信息</h3>
      </div>
      <div className="space-y-3 text-sm">
        <div className="flex justify-between">
          <span className="text-muted-foreground">状态</span>
          <span className={connected ? 'text-green-600' : 'text-muted-foreground'}>
            {connected ? '已连接' : '未连接'}
          </span>
        </div>
        <div className="flex justify-between">
          <span className="text-muted-foreground">固件版本</span>
          <span>{version || '-'}</span>
        </div>
        <div className="flex justify-between">
          <span className="text-muted-foreground">BLE 地址</span>
          <span className="font-mono text-xs">{bleAddress || '-'}</span>
        </div>
      </div>
    </div>
  )
}
