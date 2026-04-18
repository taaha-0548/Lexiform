import { describe, it, expect, vi } from 'vitest';
import { LexiformEngine } from './index';

describe('LexiformEngine', () => {
  it('should be defined', () => {
    expect(LexiformEngine).toBeDefined();
  });

  it('should have initWasm and parse methods', () => {
    expect(typeof LexiformEngine.initWasm).toBe('function');
    expect(typeof LexiformEngine.parse).toBe('function');
  });

  it('should validate correctly', () => {
    const schema: any = {
      sections: [{
        fields: [{
          id: 'test',
          label: 'Test',
          type: 'text',
          REQUIRED: true
        }]
      }]
    };
    const errors = LexiformEngine.validate(schema, { test: '' });
    expect(errors.test).toBe('Test is required.');
  });
});
