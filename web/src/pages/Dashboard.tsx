import { useEffect } from 'react'
import { useConnectionStore } from '../stores/connection'
import { useDriveInfo } from '../hooks/useDriveInfo'
import DeviceInfoCard from '../components/DeviceInfoCard'
import StorageChart from '../components/StorageChart'
import * as proto from '../lib/pixl.proto'

export default function Dashboard() {
  const { connected } = useConnectionStore()
  const { drives, fetchDrives } = useDriveInfo()

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

  const handleEnterDFU = async () => {
    if (confirm('是否进入DFU模式？')) {
      await proto.enter_dfu()
      if (confirm('设备已进入DFU模式，是否跳转到固件更新页面？')) {
        window.location.href = 'https://thegecko.github.io/web-bluetooth-dfu/examples/web.html'
      }
    }
  }

  if (!connected) {
    return (
      <div className="flex flex-col items-center justify-center h-full text-center">
        <div className="mb-6">
          <h1 className="text-3xl font-bold mb-2">Pixl.js</h1>
          <p className="text-muted-foreground">连接设备以开始管理文件</p>
        </div>
        <button
          onClick={() => useConnectionStore.getState().connect()}
          className="inline-flex items-center gap-2 px-8 py-3 rounded-lg bg-primary text-primary-foreground hover:bg-primary/90 text-base font-medium"
        >
          连接设备
        </button>
      </div>
    )
  }

  return (
    <div className="space-y-6">
      <h2 className="text-2xl font-bold">仪表盘</h2>
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
        <h3 className="font-medium text-sm mb-3">快捷操作</h3>
        <div className="flex gap-3">
          <button
            onClick={handleEnterDFU}
            className="px-4 py-2 rounded-md bg-destructive text-destructive-foreground text-sm hover:bg-destructive/90"
          >
            进入 DFU 模式
          </button>
          <button
            onClick={fetchDrives}
            className="px-4 py-2 rounded-md bg-secondary text-secondary-foreground text-sm hover:bg-secondary/80"
          >
            刷新
          </button>
        </div>
      </div>
    </div>
  )
}
