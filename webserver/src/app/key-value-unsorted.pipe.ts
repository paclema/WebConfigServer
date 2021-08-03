import { Pipe, PipeTransform } from '@angular/core';

@Pipe({
  name: 'keyValueUnsorted',
  pure: false   // To reload every new object changes
})
export class KeyValueUnsortedPipe implements PipeTransform {

  // transform(value: unknown, ...args: unknown[]): unknown {
  //   return null;
  // }
  transform(input: any): any {
    let keys = [];
    for (let key in input) {
      if (input.hasOwnProperty(key)) {
        keys.push({ key: key, value: input[key]});
      }
    }
    return keys;
  }
}
