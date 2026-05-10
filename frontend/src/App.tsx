import { useEffect, useMemo, useState } from 'react'
import './App.css'
import { LexiformEngine } from 'lexiform-published'
import { CompilerStages } from './CompilerStages'

type SchemaField = {
  id: string
  label: string
  type: string
  REQUIRED?: boolean
  OPTIONS?: string[] | string
  PLACEHOLDER?: string
}

type SchemaSection = {
  title: string
  fields: SchemaField[]
}

type SchemaResult = {
  title?: string
  id?: string
  sections: SchemaSection[]
  submit?: { label?: string }
}

const defaultSource = `FORM "Job Application" job-app-form
SECTION "Personal Information"
  TEXT "Full Name" fullName [REQUIRED]
  TEXT "Email Address" email [REQUIRED]
  TEXT "Email Address Backup" emailBackup [REQUIRED]
  TEXT "Phone Number" phone [REQUIRED]
  TEXT "Phone Number Backup" phoneBackup [REQUIRED]
  EMAIL "Verified Email" verifiedEmail [REQUIRED]
  EMAIL "Work Email" workEmail [REQUIRED]
SECTION "Professional Experience"
  TEXTAREA "Current Position" currentPosition [REQUIRED]
  TEXTAREA "Company Name" company [REQUIRED]
  TEXT "Years of Experience" yearsExp [REQUIRED]
  TEXT "Years of Experience (Backup)" yearsExpBackup [REQUIRED]
  DROPDOWN "Experience Level" expLevel [OPTIONS="Junior,Mid,Senior,Lead,Manager"]
  DROPDOWN "Experience Level Alt" expLevelAlt [OPTIONS="Junior,Mid,Senior,Lead,Manager"]
SECTION "Skills"
  CHECKBOX "Technical Skills" techSkills [OPTIONS="JavaScript,Python,Java,Go,Rust"]
  CHECKBOX "Technical Skills List" techSkillsList [OPTIONS="JavaScript,Python,Java,Go,Rust"]
  RADIO "Preferred Language" prefLang [OPTIONS="JavaScript,Python,Java,Go"]
  RADIO "Primary Language" primaryLang [OPTIONS="JavaScript,Python,Java,Go"]
SECTION "Availability"
  TEXT "Start Date" startDate [REQUIRED]
  TEXT "Available From" availableFrom [REQUIRED]
  DROPDOWN "Notice Period" noticePeriod [OPTIONS="Immediate,2 weeks,1 month,2 months"]
  DROPDOWN "Notice Required" noticeRequired [OPTIONS="Immediate,2 weeks,1 month,2 months"]
SECTION "Additional"
  TEXTAREA "Cover Letter" coverLetter [REQUIRED]
  TEXTAREA "Motivation" motivation [REQUIRED]
  TEXT "Referral Source" referral [REQUIRED]
  TEXT "Referral Code" referralCode [REQUIRED]
SUBMIT "Submit Application"
END`

function App() {
  const [isReady, setIsReady] = useState(false)
  const [loadingError, setLoadingError] = useState('')
  const [source, setSource] = useState(defaultSource)
  const [schema, setSchema] = useState<SchemaResult | null>(null)
  const [compileError, setCompileError] = useState('')
  const [formData, setFormData] = useState<Record<string, unknown>>({})
  const [validationErrors, setValidationErrors] = useState<Record<string, string>>({})
  const [showStages, setShowStages] = useState(false)

  const getFieldOptions = (field: SchemaField): string[] => {
    if (Array.isArray(field.OPTIONS)) {
      return field.OPTIONS
    }

    if (typeof field.OPTIONS === 'string') {
      return field.OPTIONS
        .split(',')
        .map((option) => option.trim())
        .filter(Boolean)
    }

    return []
  }

  const parseAndSetSchema = (dslSource: string) => {
    try {
      const nextSchema = LexiformEngine.parse(dslSource) as SchemaResult
      setSchema(nextSchema)
      setCompileError('')
      setFormData({})
      setValidationErrors({})
    } catch (error) {
      setCompileError(error instanceof Error ? error.message : 'Compilation failed.')
      setSchema(null)
      setValidationErrors({})
    }
  }

  useEffect(() => {
    let mounted = true

    void (async () => {
      try {
        await LexiformEngine.initWasm()
        if (mounted) {
          setIsReady(true)
          setLoadingError('')
          parseAndSetSchema(source)
        }
      } catch (error) {
        if (mounted) {
          setLoadingError(error instanceof Error ? error.message : 'Failed to initialize Lexiform.')
        }
      }
    })()

    return () => {
      mounted = false
    }
    // eslint-disable-next-line react-hooks/exhaustive-deps
  }, [])

  const hasSchema = useMemo(() => Boolean(schema && schema.sections.length > 0), [schema])

  const compileSource = () => {
    if (!isReady) {
      return
    }

    parseAndSetSchema(source)
  }

  const onFieldChange = (field: SchemaField, nextValue: unknown) => {
    setFormData((prev) => ({ ...prev, [field.id]: nextValue }))
    setValidationErrors((prev) => {
      if (!prev[field.id]) {
        return prev
      }

      const next = { ...prev }
      delete next[field.id]
      return next
    })
  }

  const submitMock = (event: React.FormEvent) => {
    event.preventDefault()
    if (!schema) {
      return
    }

    const errors = LexiformEngine.validate(schema as never, formData as never)
    setValidationErrors(errors)

    if (Object.keys(errors).length === 0) {
      window.alert('Mock submit passed. See console for payload.')
      console.log('Lexiform mock submit payload:', formData)
    }
  }

  return (
    <div className="mock-root">
      <div className="mock-aura" />
      <main className="mock-shell">
        <header className="mock-header">
          <p className="kicker">Lexiform</p>
          <h1>NPM Package Mock Frontend</h1>
          <p className="subcopy">Compile DSL, render fields, and validate form data in one screen.</p>
          <div className="view-toggle">
            <button
              className={`toggle-btn ${!showStages ? 'active' : ''}`}
              onClick={() => setShowStages(false)}
            >
              Form View
            </button>
            <button
              className={`toggle-btn ${showStages ? 'active' : ''}`}
              onClick={() => setShowStages(true)}
            >
              Compiler Stages
            </button>
          </div>
        </header>

        {showStages ? (
          // Compiler Stages View
          <section className="stages-view">
            <article className="panel panel-editor">
              <div className="panel-top">
                <h2>Source DSL</h2>
                <button type="button" onClick={compileSource} disabled={!isReady}>
                  {isReady ? 'Compile Form' : 'Initializing...'}
                </button>
              </div>
              <textarea
                value={source}
                onChange={(event) => setSource(event.target.value)}
                spellCheck={false}
                aria-label="Lexiform source editor"
              />
              {loadingError && <p className="status error">Init error: {loadingError}</p>}
              {compileError && <p className="status error">Compile error: {compileError}</p>}
              {!loadingError && !compileError && <p className="status ok">Engine ready: {String(isReady)}</p>}
            </article>

            <CompilerStages source={source} onFinalSchema={(schema: unknown) => setSchema(schema as SchemaResult)} />
          </section>
        ) : (
          // Form View
          <section className="mock-grid">
            <article className="panel panel-editor">
              <div className="panel-top">
                <h2>Source DSL</h2>
                <button type="button" onClick={compileSource} disabled={!isReady}>
                  {isReady ? 'Compile Form' : 'Initializing...'}
                </button>
              </div>
              <textarea
                value={source}
                onChange={(event) => setSource(event.target.value)}
                spellCheck={false}
                aria-label="Lexiform source editor"
              />
              {loadingError && <p className="status error">Init error: {loadingError}</p>}
              {compileError && <p className="status error">Compile error: {compileError}</p>}
              {!loadingError && !compileError && <p className="status ok">Engine ready: {String(isReady)}</p>}
            </article>

            <article className="panel panel-form">
              <div className="panel-top">
                <h2>{schema?.title ?? 'Preview Form'}</h2>
                <span className="pill">{hasSchema ? 'Compiled' : 'Waiting'}</span>
              </div>

              {!hasSchema && <p className="empty">No schema yet. Click Compile Form, or wait for auto-compile on startup.</p>}

              {schema && (
                <form onSubmit={submitMock} className="form-stack">
                  {schema.sections.map((section) => (
                    <section key={section.title} className="section-block">
                      <h3>{section.title}</h3>
                      {section.fields.map((field) => (
                        <label key={field.id} className="field-block">
                          <span>
                            {field.label}
                            {field.REQUIRED ? ' *' : ''}
                          </span>

                          {field.type === 'text' && (
                            <input
                              type="text"
                              placeholder={field.PLACEHOLDER ?? ''}
                              value={String(formData[field.id] ?? '')}
                              onChange={(event) => onFieldChange(field, event.target.value)}
                            />
                          )}

                          {field.type === 'email' && (
                            <input
                              type="email"
                              placeholder={field.PLACEHOLDER ?? 'you@company.com'}
                              value={String(formData[field.id] ?? '')}
                              onChange={(event) => onFieldChange(field, event.target.value)}
                            />
                          )}

                          {field.type === 'textarea' && (
                            <textarea
                              placeholder={field.PLACEHOLDER ?? ''}
                              value={String(formData[field.id] ?? '')}
                              onChange={(event) => onFieldChange(field, event.target.value)}
                            />
                          )}

                          {field.type === 'dropdown' && (
                            <select
                              value={String(formData[field.id] ?? '')}
                              onChange={(event) => onFieldChange(field, event.target.value)}
                            >
                              <option value="">Select...</option>
                              {getFieldOptions(field).map((option) => (
                                <option key={option} value={option}>
                                  {option}
                                </option>
                              ))}
                            </select>
                          )}

                          {field.type === 'radio' && (
                            <div className="options-inline">
                              {getFieldOptions(field).map((option) => (
                                <label key={option}>
                                  <input
                                    type="radio"
                                    name={field.id}
                                    checked={formData[field.id] === option}
                                    onChange={() => onFieldChange(field, option)}
                                  />
                                  {option}
                                </label>
                              ))}
                            </div>
                          )}

                          {field.type === 'checkbox' && (
                            <div className="options-inline">
                              {getFieldOptions(field).map((option) => {
                                const current = Array.isArray(formData[field.id]) ? (formData[field.id] as string[]) : []
                                const checked = current.includes(option)

                                return (
                                  <label key={option}>
                                    <input
                                      type="checkbox"
                                      checked={checked}
                                      onChange={(event) => {
                                        const next = event.target.checked
                                          ? [...current, option]
                                          : current.filter((item) => item !== option)
                                        onFieldChange(field, next)
                                      }}
                                    />
                                    {option}
                                  </label>
                                )
                              })}
                            </div>
                          )}

                          {validationErrors[field.id] && <small>{validationErrors[field.id]}</small>}
                        </label>
                      ))}
                    </section>
                  ))}

                  <button type="submit" className="submit-btn">
                    {schema.submit?.label ?? 'Submit'}
                  </button>
                </form>
              )}
            </article>

            <article className="panel panel-json">
              <div className="panel-top">
                <h2>Schema JSON</h2>
                <span className="pill">Live</span>
              </div>
              <pre>{schema ? JSON.stringify(schema, null, 2) : 'No schema yet.'}</pre>
            </article>
          </section>
        )}
      </main>
    </div>
  )
}

export default App
