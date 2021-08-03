import { Injectable } from '@angular/core';
import { HttpClient, HttpErrorResponse } from '@angular/common/http';
import { throwError } from 'rxjs';
import { catchError } from 'rxjs/operators';

@Injectable({
  providedIn: 'root'
})
export class EnrollmentService {

  // _url = 'config/temp.json';
  _url = 'http://localhost:3000/enroll';

  constructor(private _http: HttpClient) { }

  enroll(dataToEnroll){
    return this._http.post<any>(this._url, dataToEnroll)
                        .pipe(
                          catchError(this.errorHandler));
  }

  errorHandler(error: HttpErrorResponse){
    return throwError(error.message || "Error posting data")
  }


}
