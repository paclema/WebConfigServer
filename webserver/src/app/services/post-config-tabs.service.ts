import { Injectable } from '@angular/core';
import { HttpClient, HttpErrorResponse, HttpParams  } from '@angular/common/http';
import { throwError } from 'rxjs';
import { catchError } from 'rxjs/operators';

import { environment } from '../../environments/environment';


@Injectable({
  providedIn: 'root'
})
export class PostConfigTabsService {

  _url = 'http://localhost:3000/enroll';
  // _urlBase = (environment.apiUrl ? environment.apiUrl : "http://"+location.hostname) 
  //             + ":" +environment.apiPort;
  _urlBase = (environment.apiUrl ? environment.apiUrl + ":" +environment.apiPort : "http://"+location.host);
  

  constructor(private _http: HttpClient) { }

  register(dataPost){
    // return this._http.post<any>(this._url, dataPost);

    return this._http.post<any>(this._url, dataPost)
                        .pipe(
                          catchError(this.errorHandler));

  }

  errorHandler(error: HttpErrorResponse){
    return throwError(error.message || "Error posting data")
  }

  saveConfig(dataPost){
    const urlTemp = this._urlBase + "/save_config";
    return this._http.post<any>(urlTemp, dataPost)
                        .pipe(
                          catchError(this.errorHandler));

  }

  restoreBackup(filename){
    const urlTemp = this._urlBase + "/restore_config";
    const params = new HttpParams()
                  .set('filename', filename);

    return this._http.post<any>(urlTemp, params)
                        .pipe(
                          catchError(this.errorHandler));

  }

  restartDevice(){
    const urlTemp = this._urlBase + "/restart";
    const params = new HttpParams()
                  .set('restart', 'true');

    return this._http.post<any>(urlTemp, params)
                        .pipe(
                          catchError(this.errorHandler));

  }

  gpioTest(id, val){
    const urlTemp = this._urlBase + "/gpio";
    const params = new HttpParams()
                  .set('id', id)
                  .set('val', val);

    return this._http.post<any>(urlTemp, params)
                        .pipe(
                          catchError(this.errorHandler));

  }

  uploadFile(dataPost){
    const urlTemp = this._urlBase + "/uploadFile";
    return this._http.post<any>(urlTemp, dataPost, {
                reportProgress: true,
                observe: 'events'
                }).pipe(catchError(this.errorHandler));

  }

}
