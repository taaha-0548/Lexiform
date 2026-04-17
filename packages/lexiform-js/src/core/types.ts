import { FieldType } from './constants';

export interface Field {
  id: string;
  label: string;
  type: FieldType;
  REQUIRED?: boolean;
  PLACEHOLDER?: string;
  OPTIONS?: string[];
  MAX_WORDS?: number;
  MIN?: number;
  MAX?: number;
}

export interface Section {
  title: string;
  fields: Field[];
}

export interface FormSchema {
  title: string;
  id: string;
  sections: Section[];
  submit: {
    label: string;
  };
}

export type FormDataValue = string | string[] | number | boolean | undefined;
