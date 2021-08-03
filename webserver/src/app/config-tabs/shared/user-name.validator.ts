import { AbstractControl,ValidatorFn }  from '@angular/forms';

// export function ForbiddenNameValidator(control: AbstractControl):{ [key: string]: any} | null {
//   // Receives a FormControl (in particular an AbstractControl) parameter
//   // To use this validator, call it on a FormBuilder object like this:
//   // registrationForm = this.fb.group({
//   //   userName: ['', [Validators.required, Validators.minLength(3), forbiddenNameValidator]],
//   // });
//   const forbidden = /admin/.test(control.value);
//   return forbidden ? {'forbiddenName': { value: control.value}} : null;
// }

// In order to make accesible the aboved function to accept other input parameters,
// instead to hardcode "admin" string, we create a Factory fucntion that accepts
// a string as a secondf parameter and returns the validate function itself:
export function ForbiddenNameValidator(forbiddenName: RegExp): ValidatorFn {
  // To use this validator, call it on a FormBuilder object like this:
  // registrationForm = this.fb.group({
  //   userName: ['', [Validators.required, Validators.minLength(3), forbiddenNameValidator(/admin/)]],
  // });
  return (control: AbstractControl):{ [key: string]: any} | null => {
    // Receives a FormControl (in particular an AbstractControl) parameter
    const forbidden = forbiddenName.test(control.value);
    return forbidden ? {'forbiddenName': { value: control.value}} : null;
  }
}
