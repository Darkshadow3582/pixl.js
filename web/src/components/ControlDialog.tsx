import { useRef, useCallback, useState } from 'react'
import { useTranslation } from 'react-i18next'
import { useConnectionStore } from '../stores/connection'
import Dialog from './Dialog'
import * as proto from '../lib/pixl.proto'

interface ControlDialogProps {
  open: boolean
  onClose: () => void
}

export default function ControlDialog({ open, onClose }: ControlDialogProps) {
  const { t } = useTranslation()
  const { connected } = useConnectionStore()
  const canvasRef = useRef<HTMLCanvasElement>(null)
  const [refreshing, setRefreshing] = useState(false)

  const refreshScreen = useCallback(async () => {
    if (!connected) return
    setRefreshing(true)
    try {
      const d = await proto.get_scrren_buffer()
      const buffer = new Int8Array(d.data.toArrayBuffer())
      const canvas = canvasRef.current
      if (!canvas) return
      const ctx = canvas.getContext('2d')
      if (!ctx) return

      const width = 128
      const height = 64
      const imageData = ctx.createImageData(width * 2, height * 2)

      const drawPixel = (img: ImageData, x: number, y: number, bit: number) => {
        const pixelIndex = (y * width * 2 + x) * 4
        img.data[pixelIndex] = bit ? 255 : 0
        img.data[pixelIndex + 1] = bit ? 255 : 0
        img.data[pixelIndex + 2] = bit ? 255 : 0
        img.data[pixelIndex + 3] = 255
      }

      for (let x = 0; x < width; x++) {
        for (let y = 0; y < height; y++) {
          const columnIndex = x
          const bitIndex = y % 8
          const byte = buffer[columnIndex + Math.floor(y / 8) * width]
          const bit = (byte >> bitIndex) & 1

          drawPixel(imageData, x * 2, y * 2, bit)
          drawPixel(imageData, x * 2 + 1, y * 2, bit)
          drawPixel(imageData, x * 2, y * 2 + 1, bit)
          drawPixel(imageData, x * 2 + 1, y * 2 + 1, bit)
        }
      }

      ctx.putImageData(imageData, 0, 0)
    } finally {
      setRefreshing(false)
    }
  }, [connected])

  const sendKey = useCallback((key: number, type: number) => {
    if (!connected) return
    proto.send_key_event(key, type)
  }, [connected])

  return (
    <Dialog open={open} title={t('control.title')} onClose={onClose}>
      <div className="space-y-4">
        <div className="flex justify-center">
          <canvas
            ref={canvasRef}
            width={256}
            height={128}
            className="border bg-black rounded"
            style={{ width: 256, height: 128 }}
          />
        </div>
        <div className="flex justify-center gap-3">
          <button
            onClick={() => sendKey(2, 2)}
            disabled={!connected}
            className="w-10 h-10 rounded-full bg-secondary text-secondary-foreground hover:bg-secondary/80 disabled:opacity-50 flex items-center justify-center"
            title={t('control.left')}
          >
            ←
          </button>
          <button
            onClick={() => sendKey(1, 2)}
            disabled={!connected}
            className="w-10 h-10 rounded-full bg-secondary text-secondary-foreground hover:bg-secondary/80 disabled:opacity-50 flex items-center justify-center"
            title={t('control.center')}
          >
            ●
          </button>
          <button
            onClick={() => sendKey(0, 2)}
            disabled={!connected}
            className="w-10 h-10 rounded-full bg-secondary text-secondary-foreground hover:bg-secondary/80 disabled:opacity-50 flex items-center justify-center"
            title={t('control.right')}
          >
            →
          </button>
        </div>
        <div className="flex justify-center">
          <button
            onClick={refreshScreen}
            disabled={!connected || refreshing}
            className="px-4 py-2 rounded-md bg-primary text-primary-foreground text-sm hover:bg-primary/90 disabled:opacity-50"
          >
            {refreshing ? t('status.loading') : t('control.refresh')}
          </button>
        </div>
      </div>
    </Dialog>
  )
}
