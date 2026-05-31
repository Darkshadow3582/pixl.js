import React from 'react'
import ReactDOM from 'react-dom/client'
import App from './App'
import './index.css'

// i18n init must complete before React renders
const mount = () => {
  const root = document.getElementById('root')
  if (!root) {
    document.body.innerHTML = '<h2 style="color:red">Error: #root element not found</h2>'
    return
  }
  try {
    ReactDOM.createRoot(root).render(
      <React.StrictMode>
        <App />
      </React.StrictMode>,
    )
  } catch (e) {
    root.innerHTML = `<pre style="color:red;padding:40px;font-family:monospace">${e instanceof Error ? e.stack : String(e)}</pre>`
  }
}

// Handle global errors
window.onerror = (_msg, _src, _line, _col, error) => {
  document.body.innerHTML = `<pre style="color:red;padding:40px;font-family:monospace">${error?.stack || String(error) || String(_msg)}</pre>`
}

// Import i18n and mount
import('./i18n').then(mount).catch(e => {
  document.body.innerHTML = `<pre style="color:red;padding:40px;font-family:monospace">i18n init failed:\n${e.stack}</pre>`
})
