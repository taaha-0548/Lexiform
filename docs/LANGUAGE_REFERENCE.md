# Lexiform Language Reference Manual

**Version 0.1.0**  
**Authors:** Lexiform Team  
**Date:** May 2026

---

## 1. Introduction

Lexiform is a domain-specific markup language (DSL) for defining web forms. It provides a human-readable, version-controllable alternative to JSON schemas for form definition. The Lexiform compiler reads `.form` source files and produces structured JSON output that can be consumed by any frontend framework.

---

## 2. File Format

- **Extension:** `.form`
- **Encoding:** UTF-8
- **Whitespace:** Indentation is not significant; used for readability only
- **Comments:** Not currently supported (planned for v0.2.0)
- **Case sensitivity:** Keywords are UPPERCASE; identifiers are case-sensitive

---

## 3. Program Structure

Every `.form` file follows this structure:

```
FORM "<title>" <form-id>
  SECTION "<section-title>"
    <field-declarations...>
  [SECTION "<another-section>" ...]
SUBMIT "<button-label>"
END
```

### Rules
1. A form must begin with `FORM` and end with `END`
2. There must be at least **one** `SECTION`
3. Each section must contain at least **one** field
4. There must be exactly **one** `SUBMIT` declaration before `END`

---

## 4. Complete Grammar (BNF)

```bnf
<program>       ::= <form>

<form>          ::= FORM <string> <identifier> <section-list> SUBMIT <string> END

<section-list>  ::= <section> | <section> <section-list>

<section>       ::= SECTION <string> <field-list>

<field-list>    ::= <field> | <field> <field-list>

<field>         ::= <field-type> <string> <identifier> [ "[" <attr-list> "]" ]

<field-type>    ::= TEXT | EMAIL | PHONE | NUMBER | DATE 
                  | TEXTAREA | DROPDOWN | RADIO | CHECKBOX | FILE

<attr-list>     ::= <attribute> | <attribute> "," <attr-list>

<attribute>     ::= <attr-name>                         (* flag attribute *)
                  | <attr-name> "=" <attr-value>         (* key-value attribute *)

<attr-name>     ::= REQUIRED | PLACEHOLDER | OPTIONS | MIN | MAX | MAX_WORDS

<attr-value>    ::= <string> | <number> | <boolean> | <list>

<list>          ::= "[" <list-items> "]"
<list-items>    ::= <string> | <string> "," <list-items>

<string>        ::= '"' <characters> '"'
<identifier>    ::= [a-zA-Z_-][a-zA-Z0-9_-]*
<number>        ::= [0-9]+
<boolean>       ::= true | false
```

---

## 5. Keywords

| Keyword | Category | Description |
|---------|----------|-------------|
| `FORM` | Structure | Begins a form definition |
| `SECTION` | Structure | Groups related fields under a heading |
| `SUBMIT` | Structure | Defines the submit button label |
| `END` | Structure | Marks the end of the form |
| `TEXT` | Field Type | Single-line text input |
| `EMAIL` | Field Type | Email address input |
| `PHONE` | Field Type | Phone number input |
| `NUMBER` | Field Type | Numeric input (supports MIN/MAX) |
| `DATE` | Field Type | Date picker input |
| `TEXTAREA` | Field Type | Multi-line text input (supports MAX_WORDS) |
| `DROPDOWN` | Field Type | Select dropdown (requires OPTIONS) |
| `RADIO` | Field Type | Radio button group (requires OPTIONS) |
| `CHECKBOX` | Field Type | Checkbox group (requires OPTIONS) |
| `FILE` | Field Type | File upload input |

---

## 6. Field Types — Detailed

### TEXT
Single-line text input for short strings.
```
TEXT "Full Name" name [REQUIRED, PLACEHOLDER="Enter your name"]
```
**Compatible attributes:** REQUIRED, PLACEHOLDER

### EMAIL
Email address input with format validation in the rendered form.
```
EMAIL "Email" email [REQUIRED]
```
**Compatible attributes:** REQUIRED, PLACEHOLDER

### PHONE
Phone number input.
```
PHONE "Phone" phone [PLACEHOLDER="+1 (555) 000-0000"]
```
**Compatible attributes:** REQUIRED, PLACEHOLDER

### NUMBER
Numeric input with optional range constraints.
```
NUMBER "Age" age [MIN=13, MAX=120, REQUIRED]
```
**Compatible attributes:** REQUIRED, PLACEHOLDER, MIN, MAX

### DATE
Date picker input.
```
DATE "Birthday" dob
```
**Compatible attributes:** REQUIRED

### TEXTAREA
Multi-line text input with optional word limit.
```
TEXTAREA "Description" desc [MAX_WORDS=500, PLACEHOLDER="Write here..."]
```
**Compatible attributes:** REQUIRED, PLACEHOLDER, MAX_WORDS

### DROPDOWN
Single-selection dropdown list. **Requires OPTIONS.**
```
DROPDOWN "Country" country [OPTIONS=["USA", "UK", "Pakistan"]]
```
**Compatible attributes:** REQUIRED, OPTIONS

### RADIO
Single-selection radio button group. **Requires OPTIONS.**
```
RADIO "Gender" gender [OPTIONS=["Male", "Female", "Other"]]
```
**Compatible attributes:** REQUIRED, OPTIONS

### CHECKBOX
Multi-selection checkbox group.
```
CHECKBOX "Skills" skills [OPTIONS=["C++", "Python", "JavaScript"]]
```
**Compatible attributes:** OPTIONS

### FILE
File upload field.
```
FILE "Resume" resume [REQUIRED]
```
**Compatible attributes:** REQUIRED

---

## 7. Attributes Reference

| Attribute | Type | Applies To | Description |
|-----------|------|------------|-------------|
| `REQUIRED` | Flag (boolean) | All fields | Field must be filled before submission |
| `PLACEHOLDER` | String | TEXT, EMAIL, PHONE, NUMBER, TEXTAREA | Hint text shown when field is empty |
| `OPTIONS` | List of strings | DROPDOWN, RADIO, CHECKBOX | Available choices for selection fields |
| `MIN` | Integer | NUMBER | Minimum allowed value |
| `MAX` | Integer | NUMBER | Maximum allowed value |
| `MAX_WORDS` | Integer | TEXTAREA | Maximum word count allowed |

### Attribute Syntax

**Flag attributes** (no value):
```
[REQUIRED]
```

**Key-value attributes:**
```
[PLACEHOLDER="Enter text here"]
[MIN=0, MAX=100]
```

**List attributes:**
```
[OPTIONS=["Option A", "Option B", "Option C"]]
```

**Combined:**
```
[REQUIRED, PLACEHOLDER="Choose one", OPTIONS=["A", "B", "C"]]
```

---

## 8. Semantic Rules

The compiler enforces the following semantic constraints:

1. **Unique IDs:** All form and field IDs must be globally unique within a single `.form` file.
2. **Attribute compatibility:** Certain attributes are only valid on specific field types:
   - `MAX_WORDS` is only valid on `TEXTAREA`
   - `MIN` / `MAX` are only valid on `NUMBER`
   - `OPTIONS` is only valid on `DROPDOWN`, `RADIO`, and `CHECKBOX`
3. **Mandatory attributes:** `DROPDOWN` and `RADIO` fields **must** include an `OPTIONS` attribute.
4. **Submit requirement:** Every form must include a `SUBMIT` declaration.

Violating any of these rules produces a **Semantic Error** with the offending field ID and a descriptive message.

---

## 9. Error Messages

The compiler provides errors with line and column information:

| Error Type | Example Message |
|-----------|-----------------|
| Lexer Error | `Lexer error at line 3, column 5: unexpected character '@'` |
| Parser Error | `Parser error at line 4: Expected 'SECTION'` |
| Semantic Error | `Semantic Error: Duplicate ID 'email' found in FIELD` |
| Semantic Error | `Semantic Error: Field 'title' is not a TEXTAREA but uses MAX_WORDS` |

---

## 10. Output Format

The compiler produces a JSON schema with the following structure:

```json
{
  "title": "Form Title",
  "id": "form-id",
  "sections": [
    {
      "title": "Section Title",
      "fields": [
        {
          "type": "text",
          "label": "Field Label",
          "id": "field-id",
          "REQUIRED": true,
          "PLACEHOLDER": "hint text"
        }
      ]
    }
  ],
  "submit": {
    "label": "Submit Button Text"
  }
}
```

---

## 11. CLI Usage

```bash
# Basic compilation
lexiform input.form -o output.json

# Show all intermediate representations (debug mode)
lexiform input.form --debug

# Interactive REPL mode
lexiform --interactive

# Dump AST only (legacy)
lexiform input.form --ast
```

---

## 12. Example Programs

### Example 1: Contact Form
```lexiform
FORM "Contact Us" contact-form
SECTION "Identity"
    TEXT "Full Name" name [REQUIRED, PLACEHOLDER="Enter your name"]
    EMAIL "Email" email [REQUIRED]
SUBMIT "Send Message"
END
```

### Example 2: Job Application
```lexiform
FORM "Job Application" job-app
SECTION "Personal Information"
    TEXT "Full Name" name [REQUIRED]
    EMAIL "Email Address" email [REQUIRED]
    PHONE "Phone Number" phone
    DATE "Date of Birth" dob
SECTION "Experience"
    TEXTAREA "Bio" bio [MAX_WORDS=500]
    DROPDOWN "Position" pos [OPTIONS=["Engineer", "Manager", "Designer"]]
SUBMIT "Apply Now"
END
```

### Example 3: Survey
```lexiform
FORM "User Survey" survey-2026
SECTION "Experience"
    TEXTAREA "Detailed Feedback" feedback [MAX_WORDS=100]
    DROPDOWN "Rating" rating [REQUIRED, OPTIONS=["Excellent", "Good", "Fair", "Poor"]]
SECTION "Demographics"
    NUMBER "Age" age [MIN=13, MAX=120]
    RADIO "Gender" gender [OPTIONS=["Male", "Female", "Non-binary", "Prefer not to say"]]
SUBMIT "Submit Survey"
END
```
