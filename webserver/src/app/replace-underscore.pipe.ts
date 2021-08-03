import { Pipe, PipeTransform } from '@angular/core';

@Pipe({
  name: 'replaceUnderscore'
})
export class ReplaceUnderscorePipe implements PipeTransform {

  // transform(value: unknown, ...args: unknown[]): unknown {
  //   return null;
  // }
  transform(value: string): string {
  return value.replace(/_/g, ' ');
}

}
