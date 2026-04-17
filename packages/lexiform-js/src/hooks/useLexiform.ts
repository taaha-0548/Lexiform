import { useState, useEffect } from 'react';
import { LexiformEngine } from '../core/engine';
import { FormSchema, FormDataValue } from '../core/types';

/**
 * THE HOOK: The standard way for React developers to use Lexiform's WebAssembly Compiler.
 */
export function useLexiform(LexiformSource: string, moduleLoader?: () => Promise<any>) {
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
        await LexiformEngine.initWasm(moduleLoader);
        
        if (isMounted) {
          setIsReady(true);
          const generatedSchema = LexiformEngine.parse(LexiformSource);
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
  }, [LexiformSource, moduleLoader]);

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
    const newErrors = LexiformEngine.validate(schema, data);
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
