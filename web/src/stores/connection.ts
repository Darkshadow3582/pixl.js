import { create } from 'zustand'
import * as pixl from '../lib/pixl.ble'
import { sharedEventDispatcher } from '../lib/event'

interface ConnectionState {
  connected: boolean
  version: string | null
  bleAddress: string | null
  connecting: boolean
  connect: () => Promise<void>
  disconnect: () => void
}

export const useConnectionStore = create<ConnectionState>((set, get) => {
  const dispatcher = sharedEventDispatcher()

  dispatcher.addListener('ble_connected', () => {
    set({ connected: true, connecting: false })
  })

  dispatcher.addListener('ble_disconnected', () => {
    set({ connected: false, version: null, bleAddress: null, connecting: false })
  })

  dispatcher.addListener('ble_connect_error', () => {
    set({ connected: false, connecting: false })
  })

  return {
    connected: false,
    version: null,
    bleAddress: null,
    connecting: false,

    connect: async () => {
      set({ connecting: true })
      pixl.connect()
    },

    disconnect: () => {
      pixl.disconnect()
    },
  }
})
