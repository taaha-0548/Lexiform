/// <reference path="./lexiform-env.d.ts" />
import React from 'react';
import { useLexiform } from 'lexiform';
import youtubeFormSource from './youtube_upload.form?raw';
import './App.css';

export const App = () => {
  const {
    schema,
    isReady,
    compilerError,
    data,
    handleChange,
    validate,
    errors
  } = useLexiform(youtubeFormSource);

  if (!isReady) return <div className="loading">Initializing C++ Engine...</div>;
  if (compilerError) return <div className="error">Compiler Error: {compilerError}</div>;

  const handleSubmit = (e: React.FormEvent) => {
    e.preventDefault();
    if (validate()) {
      alert("Video Published Successfully!\nCheck console for payload.");
      console.log("YouTube Video Metadata Payload:", data);
    }
  };

  return (
    <div className="yt-container">
      <header className="yt-header">
        <div className="yt-logo">YouTube Studio</div>
        <div className="yt-avatar"></div>
      </header>

      <main className="yt-main">
        <div className="yt-dialog">
          <div className="yt-dialog-header">
            <h2>{schema?.title || "Upload Video"}</h2>
          </div>

          <div className="yt-dialog-body">
            <form id="yt-form" onSubmit={handleSubmit}>
              {schema?.sections.map((section, sIdx) => (
                <div key={sIdx} className="yt-section">
                  <h3>{section.title}</h3>
                  {section.fields.map((field) => (
                    <div key={field.id} className="yt-field">
                      <label className="yt-label" htmlFor={field.id}>
                        {field.label} {field.REQUIRED && '*'}
                      </label>
                      
                      {field.type === 'textarea' ? (
                        <textarea
                          id={field.id}
                          placeholder={field.PLACEHOLDER || ''}
                          value={(data[field.id] as string) || ''}
                          onChange={(e) => handleChange(field.id, e.target.value)}
                        />
                      ) : field.type === 'dropdown' ? (
                        <select
                          id={field.id}
                          value={(data[field.id] as string) || ''}
                          onChange={(e) => handleChange(field.id, e.target.value)}
                        >
                          <option value="">Select...</option>
                          {(Array.isArray(field.OPTIONS) ? field.OPTIONS : []).map(opt => (
                            <option key={opt} value={opt}>{opt}</option>
                          ))}
                        </select>
                      ) : field.type === 'radio' ? (
                        <div className="yt-options">
                          {(Array.isArray(field.OPTIONS) ? field.OPTIONS : []).map(opt => (
                            <label key={opt} className="yt-radio">
                              <input
                                type="radio"
                                name={field.id}
                                value={opt}
                                checked={data[field.id] === opt}
                                onChange={(e) => handleChange(field.id, e.target.value)}
                              />
                              {opt}
                            </label>
                          ))}
                        </div>
                      ) : field.type === 'checkbox' ? (
                        <div className="yt-options">
                          {(Array.isArray(field.OPTIONS) ? field.OPTIONS : []).map(opt => {
                            const current = Array.isArray(data[field.id]) ? (data[field.id] as string[]) : [];
                            return (
                              <label key={opt} className="yt-checkbox">
                                <input
                                  type="checkbox"
                                  value={opt}
                                  checked={current.includes(opt)}
                                  onChange={(e) => {
                                    const next = e.target.checked
                                      ? [...current, opt]
                                      : current.filter(x => x !== opt);
                                    handleChange(field.id, next);
                                  }}
                                />
                                {opt}
                              </label>
                            );
                          })}
                        </div>
                      ) : (
                        <input
                          id={field.id}
                          type={field.type}
                          placeholder={field.PLACEHOLDER || ''}
                          value={(data[field.id] as string) || ''}
                          onChange={(e) => handleChange(field.id, e.target.value)}
                        />
                      )}
                      
                      {errors[field.id] && (
                        <div className="yt-error">{errors[field.id]}</div>
                      )}
                    </div>
                  ))}
                </div>
              ))}
            </form>
          </div>

          <div className="yt-dialog-footer">
            <button type="submit" form="yt-form" className="yt-btn-primary">
              {schema?.submit?.label || "Publish"}
            </button>
          </div>
        </div>
      </main>
    </div>
  );
};