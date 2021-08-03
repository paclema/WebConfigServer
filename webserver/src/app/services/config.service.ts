import { Injectable } from '@angular/core';
import { HttpClient, HttpErrorResponse } from '@angular/common/http';
// import { Observable, throwError } from 'rxjs';
import { throwError } from 'rxjs';
import { catchError } from 'rxjs/operators';


@Injectable({
  providedIn: 'root'
})
export class ConfigService {

  private _urlConfigFileLocation: string = "/config/config.json"

  constructor(private http: HttpClient) { }

  // get config.json without class interface:
  getConfigData(){
    // Return the http get request as Observable to get the config.json file
    return this.http.get(this._urlConfigFileLocation)
                      .pipe(
                        catchError(this.errorHandler));
  }

  // get config.json with class interface:
  // getConfigData() Observable<IConfig[]>{
  //   // Return the http get request tp get the config.json file
  // return this.http.get(this._urlConfigFileLocation)
  //                   .pipe(
  //                     catchError(this.errorHandler));
  // }

  // To throw out a message to those who are subscribed that something went
  // wrong geting the data from the HTTTP request:
  errorHandler(error: HttpErrorResponse){
    return throwError(error.message || "Error getting config.json")
  }

}
