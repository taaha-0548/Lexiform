/// <reference path="./lexiform-env.d.ts" />
import React from 'react';
import { useLexiform } from 'lexiform';

/**
 * PRO-TIP: In Vite, you can import .form files as raw strings 
 * by appending '?raw' to the import path.
 */
import contactFormSource from './contact.form?raw';

export const FormRenderer = () => {
  const { 
    schema, 
    isReady, 
    compilerError, 
    data, 
    handleChange, 
    validate, 
    errors 
  } = useLexiform(contactFormSource);

  if (!isReady) return <div>Initializing C++ Engine...</div>;
  if (compilerError) return <div style={{color: 'red'}}>Compiler Error: {compilerError}</div>;

  const handleSubmit = (e: React.FormEvent) => {
    e.preventDefault();
    if (validate()) {
      console.log("Form Data Validated by C++ Core Logic:", data);
      // Send to your API...
    }
  };

  return (
    <form onSubmit={handleSubmit} className="lexiform-container">
      <h1>{schema?.title}</h1>
      
      {schema?.sections.map((section) => (
        <fieldset key={section.title}>
          <legend>{section.title}</legend>
          {section.fields.map((field) => (
            <div key={field.id} className="field-group">
              <label htmlFor={field.id}>{field.label}</label>
              
              {field.type === 'textarea' ? (
                <textarea
                  id={field.id}
                  value={(data[field.id] as string) || ''}
                  onChange={(e) => handleChange(field.id, e.target.value)}
                />
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
                <span className="error-text">{errors[field.id]}</span>
              )}
            </div>
          ))}
        </fieldset>
      ))}

      <button type="submit">{schema?.submit.label}</button>
    </form>
  );
};
