export const FieldType = {
  TEXT: 'text',
  EMAIL: 'email',
  PHONE: 'phone',
  NUMBER: 'number',
  DATE: 'date',
  TEXTAREA: 'textarea',
  DROPDOWN: 'dropdown',
  RADIO: 'radio',
  CHECKBOX: 'checkbox',
  FILE: 'file'
} as const;

export type FieldType = typeof FieldType[keyof typeof FieldType];
