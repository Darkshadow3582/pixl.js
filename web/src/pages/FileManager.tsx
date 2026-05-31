import { useState, useCallback, useEffect } from 'react'
import { useTranslation } from 'react-i18next'
import { useNavigate } from 'react-router-dom'
import { useConnectionStore } from '../stores/connection'
import { useFiles, FileEntry } from '../hooks/useFiles'
import * as proto from '../lib/pixl.proto'
import { appendSegment } from '../lib/utils'
import FileToolbar from '../components/FileToolbar'
import FileBreadcrumb from '../components/FileBreadcrumb'
import FileGrid from '../components/FileGrid'
import FileList from '../components/FileList'
import FileContextMenu from '../components/FileContextMenu'
import UploadDialog from '../components/UploadDialog'
import PropertyDialog from '../components/PropertyDialog'
import Dialog from '../components/Dialog'

export default function FileManager() {
  const { t } = useTranslation()
  const navigate = useNavigate()
  const { connected } = useConnectionStore()
  const { files, loading, currentDir, setCurrentDir, reloadDrive, reloadFolder, setFiles } = useFiles()
  const [viewMode, setViewMode] = useState<'grid' | 'list'>('grid')
  const [selected, setSelected] = useState<Set<string>>(new Set())
  const [searchQuery, setSearchQuery] = useState('')
  const [contextMenu, setContextMenu] = useState<{ file: FileEntry; x: number; y: number } | null>(null)
  const [uploadOpen, setUploadOpen] = useState(false)
  const [propertyFile, setPropertyFile] = useState<FileEntry | null>(null)

  const [confirmState, setConfirmState] = useState<{ msg: string; onOk: () => void } | null>(null)
  const [promptState, setPromptState] = useState<{ msg: string; value: string; onSubmit: (v: string) => void } | null>(null)
  const [alertState, setAlertState] = useState<string | null>(null)

  useEffect(() => {
    if (!connected) navigate('/')
  }, [connected, navigate])

  useEffect(() => {
    if (connected) reloadDrive()
  }, [connected, reloadDrive])

  const confirm = useCallback((msg: string) => new Promise<boolean>(resolve => {
    setConfirmState({ msg, onOk: () => { setConfirmState(null); resolve(true) } })
  }), [])

  const prompt = useCallback((msg: string, value = '') => new Promise<string | null>(resolve => {
    setPromptState({ msg, value, onSubmit: (v) => { setPromptState(null); resolve(v) } })
  }), [])

  const alert = useCallback((msg: string) => new Promise<void>(resolve => {
    setAlertState(msg)
  }), [])

  const navigateTo = useCallback((dir: string) => {
    if (!dir) {
      setCurrentDir('')
      reloadDrive()
    } else {
      setCurrentDir(dir)
      reloadFolder(dir)
    }
  }, [setCurrentDir, reloadDrive, reloadFolder])

  const handleOpen = useCallback((file: FileEntry) => {
    if (file.type === 'DRIVE') {
      const dir = file.name.substring(0, 3)
      setCurrentDir(dir)
      reloadFolder(dir)
    } else if (file.type === 'DIR') {
      const dir = currentDir.endsWith('/') ? currentDir + file.name : currentDir + '/' + file.name
      setCurrentDir(dir)
      reloadFolder(dir)
    } else {
      const path = appendSegment(currentDir, file.name)
      proto.vfs_helper_read_file(path,
        (data: ArrayBuffer) => {
          const url = URL.createObjectURL(new Blob([data]))
          const a = document.createElement('a')
          a.href = url
          a.download = file.name
          a.click()
          URL.revokeObjectURL(url)
        },
        () => {},
        () => {},
      )
    }
  }, [currentDir, setCurrentDir, reloadFolder])

  const handleNewFolder = useCallback(async () => {
    const name = await prompt(t('dialog.newfolder_message'))
    if (!name) return
    const path = appendSegment(currentDir, name)
    try {
      const res = await proto.vfs_create_folder(path)
      if (res.status !== 0) {
        await alert(t('dialog.newfolder_failed', { code: res.status }))
        return
      }
    } catch (e: any) {
      await alert(t('dialog.newfolder_failed', { code: e.message }))
      return
    }
    reloadFolder(currentDir)
  }, [currentDir, reloadFolder])

  const handleDelete = useCallback(async () => {
    if (selected.size === 0) return
    const ok = await confirm(t('dialog.delete_confirm', { count: selected.size }))
    if (!ok) return
    let failed: string[] = []
    for (const name of selected) {
      const path = appendSegment(currentDir, name)
      try {
        const res = await proto.vfs_remove(path)
        if (res.status !== 0) {
          failed.push(name + ' [' + res.status + ']')
        }
      } catch (e: any) {
        failed.push(name + ' [' + e.message + ']')
      }
    }
    if (failed.length > 0) {
      await alert(t('dialog.delete_failed', { errors: failed.join('\n') }))
    }
    setSelected(new Set())
    reloadFolder(currentDir)
  }, [selected, currentDir, reloadFolder])

  const handleRename = useCallback(async (file: FileEntry) => {
    const name = await prompt(t('dialog.rename_message'), file.name)
    if (!name || name === file.name) return
    const oldPath = appendSegment(currentDir, file.name)
    const newPath = appendSegment(currentDir, name)
    try {
      const res = await proto.vfs_rename(oldPath, newPath)
      if (res.status !== 0) {
        await alert(t('dialog.rename_failed', { code: res.status }))
        return
      }
    } catch (e: any) {
      await alert(t('dialog.rename_failed', { code: e.message }))
      return
    }
    reloadFolder(currentDir)
  }, [currentDir, reloadFolder])

  const filteredFiles = files.filter(f =>
    f.name.toLowerCase().includes(searchQuery.toLowerCase())
  )

  return (
    <div className="h-full flex flex-col">
      <h2 className="text-2xl font-bold mb-4">{t('files.title')}</h2>

      <FileToolbar
        viewMode={viewMode}
        onToggleView={() => setViewMode(v => v === 'grid' ? 'list' : 'grid')}
        onUpload={() => setUploadOpen(true)}
        onNewFolder={handleNewFolder}
        onDelete={handleDelete}
        onUp={() => navigateTo('')}
        onRefresh={() => currentDir ? reloadFolder(currentDir) : reloadDrive()}
        onSearch={setSearchQuery}
        hasSelection={selected.size > 0}
      />

      <FileBreadcrumb path={currentDir} onNavigate={navigateTo} />

      <div className="flex-1 overflow-auto">
        {loading ? (
          <div className="flex items-center justify-center h-32 text-muted-foreground">{t('files.loading')}</div>
        ) : viewMode === 'grid' ? (
          <FileGrid
            files={filteredFiles}
            selected={selected}
            onSelect={(name) => setSelected(prev => {
              const next = new Set(prev)
              next.has(name) ? next.delete(name) : next.add(name)
              return next
            })}
            onOpen={handleOpen}
            onContextMenu={(file, x, y) => setContextMenu({ file, x, y })}
          />
        ) : (
          <FileList
            files={filteredFiles}
            selected={selected}
            onSelect={(name) => setSelected(prev => {
              const next = new Set(prev)
              next.has(name) ? next.delete(name) : next.add(name)
              return next
            })}
            onOpen={handleOpen}
            onContextMenu={(file, x, y) => setContextMenu({ file, x, y })}
          />
        )}
      </div>

      {contextMenu && (
        <FileContextMenu
          x={contextMenu.x}
          y={contextMenu.y}
          onRename={() => handleRename(contextMenu.file)}
          onDelete={() => {
            setSelected(new Set([contextMenu.file.name]))
            handleDelete()
          }}
          onProperties={() => setPropertyFile(contextMenu.file)}
          onClose={() => setContextMenu(null)}
        />
      )}

      <UploadDialog
        currentDir={currentDir}
        open={uploadOpen}
        onClose={() => setUploadOpen(false)}
        onDone={() => reloadFolder(currentDir)}
      />

      <PropertyDialog
        file={propertyFile}
        currentDir={currentDir}
        open={!!propertyFile}
        onClose={() => setPropertyFile(null)}
      />

      <Dialog
        open={!!confirmState}
        title={t('dialog.confirm')}
        onClose={() => setConfirmState(null)}
        footer={
          <>
            <button onClick={() => setConfirmState(null)} className="px-4 py-2 rounded-md border text-sm">{t('dialog.cancel')}</button>
            <button onClick={() => confirmState?.onOk()} className="px-4 py-2 rounded-md bg-primary text-primary-foreground text-sm">{t('dialog.ok')}</button>
          </>
        }
      >
        <p className="text-sm whitespace-pre-wrap">{confirmState?.msg}</p>
      </Dialog>

      <Dialog
        open={!!promptState}
        title={t('dialog.input')}
        onClose={() => { setPromptState(null); promptState?.onSubmit('') }}
        footer={
          <>
            <button onClick={() => { setPromptState(null); promptState?.onSubmit('') }} className="px-4 py-2 rounded-md border text-sm">{t('dialog.cancel')}</button>
            <button onClick={() => { const input = document.getElementById('prompt-input') as HTMLInputElement; promptState?.onSubmit(input?.value || '') }} className="px-4 py-2 rounded-md bg-primary text-primary-foreground text-sm">{t('dialog.ok')}</button>
          </>
        }
      >
        <p className="text-sm mb-3">{promptState?.msg}</p>
        <input id="prompt-input" defaultValue={promptState?.value} autoFocus className="w-full h-9 rounded-md border border-input bg-background px-3 text-sm" />
      </Dialog>

      <Dialog
        open={!!alertState}
        title={t('dialog.alert')}
        onClose={() => setAlertState(null)}
        footer={
          <button onClick={() => setAlertState(null)} className="px-4 py-2 rounded-md bg-primary text-primary-foreground text-sm">{t('dialog.ok')}</button>
        }
      >
        <p className="text-sm whitespace-pre-wrap">{alertState}</p>
      </Dialog>
    </div>
  )
}
