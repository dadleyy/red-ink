// Types for compiled templates
declare module 'redink-ui/*/template' {
  import { TemplateFactory } from 'htmlbars-inline-precompile';
  const tmpl: TemplateFactory;
  export default tmpl;
}
