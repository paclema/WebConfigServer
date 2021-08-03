import { AbstractControl }  from '@angular/forms';

export function PasswordValidator(control: AbstractControl):{ [key: string]: boolean} | null {
  // Received FormGroup parameter
  const password = control.get('password');
  const confirmPassword = control.get('confirmPassword');

  // To check if both passwords were init with some values:
  if (password.pristine || confirmPassword.pristine) {
    return null;
  }
  // If password and confirmPassword are not blanck and both have the same value:
  return password && confirmPassword && password.value !== confirmPassword.value ?
    {'misMatch': true} :    // Return if they are the same password
    null;                   // Return if they are not the same
}
