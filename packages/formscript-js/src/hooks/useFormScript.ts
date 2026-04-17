import { useState, useEffect } from 'react';
import { FormScriptEngine } from '../core/engine';
import { FormSchema, FormDataValue } from '../core/types';

/**
 * THE HOOK: The standard way for React developers to use FormScript's WebAssembly Compiler.
 */
export function useFormScript(formScriptSource: string, moduleLoader?: () => Promise<any>) {
  const [data, setData] = useState<Record<string, FormDataValue>>({});
  const [errors, setErrors] = useState<Record<string, string>>({});
  const [schema, setSchema] = useState<FormSchema | null>(null);
  const [isReady, setIsReady] = useState(false);
  const [compilerError, setCompilerError] = useState<string | null>(null);

  // Initialize the C++ WebAssembly Compiler
  useEffect(() => {
    let isMounted = true;
    
    const initAndCompile = async () => {
      try {
        await FormScriptEngine.initWasm(moduleLoader);
        
        if (isMounted) {
          setIsReady(true);
          const generatedSchema = FormScriptEngine.parse(formScriptSource);
          setSchema(generatedSchema);
          setCompilerError(null);
        }
      } catch (err: any) {
        if (isMounted) {
          setCompilerError(err.message || "Failed to initialize or parse using C++ core.");
          setIsReady(false);
        }
      }
    };

    initAndCompile();

    return () => { isMounted = false; };
  }, [formScriptSource, moduleLoader]);

  const handleChange = (id: string, value: FormDataValue) => {
    setData(prev => ({ ...prev, [id]: value }));
    if (errors[id]) {
      setErrors(prev => {
        const next = { ...prev };
        delete next[id];
        return next;
      });
    }
  };

  const validate = () => {
    if (!schema) return false;
    const newErrors = FormScriptEngine.validate(schema, data);
    setErrors(newErrors);
    return Object.keys(newErrors).length === 0;
  };

  return {
    schema,
    data,
    errors,
    handleChange,
    validate,
    isValid: Object.keys(errors).length === 0,
    isReady,
    compilerError
  };
}
