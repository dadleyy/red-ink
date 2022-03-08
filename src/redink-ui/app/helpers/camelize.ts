import { helper } from '@ember/component/helper';
import { camelize } from '@ember/string';

function help([input]: [string]): string {
  return camelize(input);
}

export default helper(help);
