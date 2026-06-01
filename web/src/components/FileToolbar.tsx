import { useTranslation } from 'react-i18next'
import { FolderPlus, Upload, Trash2, Download, ArrowUp, RefreshCw, Search, LayoutGrid, List, CheckSquare } from 'lucide-react'

interface Props {
  viewMode: 'grid' | 'list'
  onToggleView: () => void
  onUpload: () => void
  onNewFolder: () => void
  onDelete: () => void
  onDownload: () => void
  onUp: () => void
  onRefresh: () => void
  onSearch: (query: string) => void
  hasSelection: boolean
  multiSelect: boolean
  onToggleMultiSelect: () => void
}

export default function FileToolbar({
  viewMode, onToggleView, onUpload, onNewFolder, onDelete, onDownload,
  onUp, onRefresh, onSearch, hasSelection, multiSelect, onToggleMultiSelect,
}: Props) {
  const { t } = useTranslation()
  return (
    <div className="flex items-center gap-2 pb-4 border-b mb-4">
      <button onClick={onUpload} className="btn-toolbar" title={t('menu.upload')}>
        <Upload className="w-4 h-4" /> {t('menu.upload')}
      </button>
      <button onClick={onNewFolder} className="btn-toolbar" title={t('menu.newfolder')}>
        <FolderPlus className="w-4 h-4" /> {t('menu.newfolder')}
      </button>
      <button onClick={onDelete} className="btn-toolbar text-destructive" title={t('menu.del')} disabled={!hasSelection}>
        <Trash2 className="w-4 h-4" /> {t('menu.del')}
      </button>
      <button onClick={onDownload} className="btn-toolbar" title={t('menu.download')} disabled={!hasSelection}>
        <Download className="w-4 h-4" /> {t('menu.download')}
      </button>
      <div className="w-px h-6 bg-border mx-1" />
      <button onClick={onUp} className="btn-toolbar" title={t('menu.up')}>
        <ArrowUp className="w-4 h-4" />
      </button>
      <button onClick={onRefresh} className="btn-toolbar" title={t('menu.refresh')}>
        <RefreshCw className="w-4 h-4" />
      </button>
      <div className="flex-1" />
      <div className="relative">
        <Search className="w-4 h-4 absolute left-3 top-1/2 -translate-y-1/2 text-muted-foreground" />
        <input
          placeholder={t('files.search_placeholder')}
          onChange={(e) => onSearch(e.target.value)}
          className="h-9 w-48 rounded-md border border-input bg-background pl-9 pr-3 text-sm"
        />
      </div>
      <button onClick={onToggleMultiSelect} className={`btn-toolbar ${multiSelect ? 'bg-accent' : ''}`} title={t('files.multi_select')}>
        <CheckSquare className="w-4 h-4" />
      </button>
      <button onClick={onToggleView} className="btn-toolbar" title={viewMode === 'grid' ? t('files.list_view') : t('files.grid_view')}>
        {viewMode === 'grid' ? <List className="w-4 h-4" /> : <LayoutGrid className="w-4 h-4" />}
      </button>
      <style>{`
  .btn-toolbar {
    display: inline-flex;
    align-items: center;
    gap: 0.375rem;
    padding: 0.375rem 0.75rem;
    font-size: 0.875rem;
    border-radius: 0.375rem;
    border: 1px solid hsl(var(--border));
    background: hsl(var(--background));
    color: hsl(var(--foreground));
    cursor: pointer;
  }
  .btn-toolbar:hover { background: hsl(var(--accent)); }
  .btn-toolbar:disabled { opacity: 0.5; cursor: not-allowed; }
`}</style>
    </div>
  )
}
