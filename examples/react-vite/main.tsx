import React from 'react'
import ReactDOM from 'react-dom/client'
import { FormRenderer } from './FormRenderer'

ReactDOM.createRoot(document.getElementById('root')!).render(
  <React.StrictMode>
    <div style={{ padding: '40px' }}>
      <FormRenderer />
    </div>
  </React.StrictMode>,
)