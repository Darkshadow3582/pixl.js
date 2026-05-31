import { Outlet } from 'react-router-dom'
import { useConnectionStore } from '../stores/connection'
import Sidebar from './Sidebar'
import TopBar from './TopBar'

export default function Layout() {
  const { connected } = useConnectionStore()

  return (
    <div className="flex h-screen">
      {connected && <Sidebar />}
      <div className="flex-1 flex flex-col">
        <TopBar />
        <main className="flex-1 overflow-auto p-6">
          <Outlet />
        </main>
      </div>
    </div>
  )
}
