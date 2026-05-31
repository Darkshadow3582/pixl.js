import { NavLink } from 'react-router-dom'
import { LayoutDashboard, FolderOpen, Settings } from 'lucide-react'
import { useConnectionStore } from '../stores/connection'

const allNavItems = [
  { to: '/', icon: LayoutDashboard, label: '仪表盘' },
  { to: '/files', icon: FolderOpen, label: '文件管理' },
  { to: '/settings', icon: Settings, label: '设置' },
]

export default function Sidebar() {
  const { connected } = useConnectionStore()
  const navItems = connected ? allNavItems : [allNavItems[0]]

  return (
    <aside className="w-56 h-screen border-r bg-card flex flex-col">
      <div className="p-4 border-b">
        <h1 className="text-lg font-bold">Pixl.js</h1>
      </div>
      <nav className="flex-1 p-2 space-y-1">
        {navItems.map((item) => (
          <NavLink
            key={item.to}
            to={item.to}
            end={item.to === '/'}
            className={({ isActive }) =>
              `flex items-center gap-3 px-3 py-2 rounded-md text-sm ${
                isActive
                  ? 'bg-primary/10 text-primary font-medium'
                  : 'text-muted-foreground hover:bg-accent'
              }`
            }
          >
            <item.icon className="w-4 h-4" />
            {item.label}
          </NavLink>
        ))}
      </nav>
    </aside>
  )
}
