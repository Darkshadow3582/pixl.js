import { useTranslation } from 'react-i18next'

interface Props {
  path: string
  onNavigate: (path: string) => void
}

export default function FileBreadcrumb({ path, onNavigate }: Props) {
  const { t } = useTranslation()
  if (!path) return null

  const segments = path.split('/').filter(Boolean)
  const paths = segments.map((_, i) => segments.slice(0, i + 1).join('/'))

  return (
    <nav className="flex items-center gap-1 text-sm mb-4">
      <button
        onClick={() => onNavigate('')}
        className="text-muted-foreground hover:text-foreground"
      >
        {t('labels.root')}
      </button>
      {segments.map((seg, i) => (
        <span key={i} className="flex items-center gap-1">
          <span className="text-muted-foreground">/</span>
          <button
            onClick={() => onNavigate(paths[i])}
            className="hover:text-foreground"
          >
            {seg}
          </button>
        </span>
      ))}
    </nav>
  )
}
