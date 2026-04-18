/// <reference types="vite/client" />

declare module '*.form?raw' {
  const content: string;
  export default content;
}

declare module '*.form' {
  const content: string;
  export default content;
}
