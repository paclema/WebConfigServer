import { Component, OnInit, ViewChild } from '@angular/core';
import { ConfigService } from '../services/config.service';
import { EnrollmentService } from '../services/enrollment.service';
import { PostConfigTabsService } from '../services/post-config-tabs.service';

// For the (manual) Reactive Form:
// import { FormGroup, FormControl } from '@angular/forms';
// For the (auto) Reactive Form:
import { FormBuilder, Validators, FormGroup, FormArray }  from '@angular/forms';
import { ForbiddenNameValidator }  from './shared/user-name.validator';
import { PasswordValidator }  from './shared/password.validator';

// Nebular:
import { 
  NbCardModule,
  NbToastrService,
  NbGlobalPosition,
  NbToastrConfig,
  NbPopoverDirective,
  NbIconConfig,
} from '@nebular/theme';

// File upload:
// import { environment } from '../../environments/environment';
// _urlBase = (environment.apiUrl ? environment.apiUrl + ":" +environment.apiPort : "http://"+location.host);
import { HttpEventType  } from '@angular/common/http';


interface FileConfig {
  nameConfig: string
  nameTab: string
  location: string
  file?: File
  uploadProgress?: number
}

@Component({
  selector: 'app-config-tabs',
  templateUrl: './config-tabs.component.html',
  styleUrls: ['./config-tabs.component.scss']
})
export class ConfigTabsComponent implements OnInit {

  public configData;
  public errorMsg;

  // For handling upload files:
  fileConfigs: FileConfig[] = [];


  configTabsForm: FormGroup;

  savingPOST = false;

  @ViewChild(NbPopoverDirective) popover: NbPopoverDirective;



  constructor(
    private _configService: ConfigService,
    private _enrollmentService: EnrollmentService,
    private fb: FormBuilder,
    private _postConfigTabsService: PostConfigTabsService,
    private toastrService: NbToastrService
    ) {

    this.configTabsForm = this.fb.group({});
  }

  ngOnInit(): void {

    // Load config.json configuration data:
    this.loadConfigFile();
    // console.log(this.configData);
          
  }


  loadConfigFile(){
    // Subscribe to the Observable received within the HTTP request to get the data
    this._configService.getConfigData()
      .subscribe(
        data =>{ 
          this.configData = data;
          this.buildTabsForm(data);
          const iconConfig: Partial<NbToastrConfig> = { icon: 'info', duration: 3000 };
          this.toastrService.info(Object.keys(data),'Configurations loaded', iconConfig);
        },
        error =>{ 
          this.toastrService.danger(error,'Error');
        }
      );
  }

  
  // configToastr: Partial<NbToastrConfig> = {
  //   position: NbGlobalPhysicalPosition.BOTTOM_LEFT,
  //   // status: "warning",
  //   duration: 10*1000
  // };

  saveConfigTabs(){
    // console.log(this.configTabsForm.value);
    this.savingPOST = true;
    this._postConfigTabsService.saveConfig(this.configTabsForm.value)
    .subscribe(
      response => {
        console.log('Success posting the data', response);
        for (const [key, value] of Object.entries(response)) {
          this.toastrService.success(response[key],key,
            // this.configToastr
            );
        }
        this.savingPOST = false;
        },
      error => {
        console.log('Error posting the data', error);
        this.toastrService.danger(error,'Error');
        this.savingPOST = false;
      }
    );

  }

  restartDevice(){
    // console.log(this.configTabsForm.value);

    this._postConfigTabsService.restartDevice()
    .subscribe(
      response => {
        console.log('Success restarting the device', response);
        for (const [key, value] of Object.entries(response)) {
          this.toastrService.success(response[key],key);
          }
        },
      error => {
        console.log('Error restarting the device', error);
        this.toastrService.danger(error,'Error');
        }
    )

  }

  gpioTest(){
    // console.log(this.configTabsForm.value);
    const id = "LED_BUILTIN";
    const val = true;

    this._postConfigTabsService.gpioTest(id, val)
    .subscribe(
      response => {
        console.log('Success testing GPIO', response);
        for (const [key, value] of Object.entries(response)) {
          this.toastrService.success(response[key],key);
          }
        },
      error => {
        console.log('Error testing GPIO', error);
        this.toastrService.danger(error,'Error');
      }
    )

  }

  configFactoryDefaults(){
    console.log("Restoring the backup for config.json...");
    // The property of this class:
    // console.log(this.testFormModel);
    // this.errorMsgPost = false;

    this.popover.hide();

    this._postConfigTabsService.restoreBackup("/config/config.json")
    .subscribe(
      response => {
        console.log('Success restoring config.json', response);
        for (const [key, value] of Object.entries(response)) {
          this.toastrService.success(response[key],key);
          }
        },
      error => {
        console.log('Error restoring config.json', error);
        this.toastrService.danger(error,'Error');
      }
    )

  }

  buildTabsForm(configTabs){
    console.log('Building configTabsForm...');
    // console.log(configTabs);
    this.configTabsForm = this.fb.group({});

    for(let tab in configTabs) {
      let newTabForm = this.fb.group({});
      for(let ind in configTabs[tab]) {
        // newTabForm.addControl(ind, this.fb.control(configTabs[tab][ind]));
        // if(this.isArray(configTabs[tab][ind]))
        // console.log('ind:'+ ind + ' -> ' + configTabs[tab][ind] + ' -> '+ this.isObject(configTabs[tab][ind]));

        // If the value is another nested object, create another FormGroup with FormControls for each key:
        if(this.isObject(configTabs[tab][ind])){
          // console.log('ind:'+ ind + ' -> ' + configTabs[tab][ind] + ' is an obejct-> '+ this.isObject(configTabs[tab][ind]));
          let nestedConfigForm = this.fb.group({});
          for(let ind2 in configTabs[tab][ind]) {
            // Create nested FormControls with validator:
            nestedConfigForm.addControl(ind2, this.fb.control(configTabs[tab][ind][ind2], Validators.required));
          }
          // Add the nested FormGroup to the tab FormGroup:
          newTabForm.addControl(ind, nestedConfigForm);

        } else {
          newTabForm.addControl(ind, this.fb.control(configTabs[tab][ind], Validators.required));
        }

        // Create file instances for each config that contains "file" name:
        if(this.isFileForm(ind)){
          // this.fileConfig.push( { nameConfig: ind, location: configTabs[tab][ind], file: null} );
          this.fileConfigs.push( { nameConfig: ind, nameTab: tab, location: configTabs[tab][ind]} );
        }

      }
      // console.log('newTabForm: ');
      // console.log(newTabForm);
      // newTabForm.setParent(this.configTabsForm);
      this.configTabsForm.addControl(tab, newTabForm);

    }
    console.log('Form built: configTabsForm');
    // console.log(this.configTabsForm);
    // console.log('Form built: configTabsForm.value');
    // console.log(this.configTabsForm.value);
    // console.log(this.fileConfigs);
  }

  // This function its added to use keyvalue pipe under *ngFor to get the
  // configTabsForm.controls unsorted:
  returnZero() {
    return 0
  }

  isNumber(val): boolean { return typeof val === 'number'; }
  isString(val): boolean { return typeof val === 'string'; }
  isBoolean(val): boolean { return typeof val === 'boolean'; }
  isArray(val): boolean {
    if (Array.isArray(val)) return true;
    return false; }
  isObject(val): boolean {
    if (Array.isArray(val)) return false
    return typeof val === 'object'; }
  isFile(key): boolean {
    if (key.includes("file")) {
      return true;
    } else
      return false;
  }

  getTypeInputForm(obj): string {
    // console.log("Receiving obj");
    // console.log(obj);

    if (this.isNumber(obj)) {
      return 'number';
    } else if (this.isString(obj)) {
      return 'string';
    } else if (this.isBoolean(obj)) {
      return 'boolean';
    } else if (this.isArray(obj)) {
      return 'array';
    } else if (this.isObject(obj)) {
      return 'object';
    } else
      return null;

  }

  isFileForm(obj): boolean {
    if (obj.includes("file")){
      return true;
    } else
      return false;
  }

  getFileConfig(nameTab, nameConfig): FileConfig {
    for(let i in this.fileConfigs) {
      if((nameTab === this.fileConfigs[i].nameTab) &&
         (nameConfig === this.fileConfigs[i].nameConfig)){
        return this.fileConfigs[i];
      }
    }
    return null;
  }


  log(val) { console.log(val); }

  prettyPrint(text) {
    // var ugly = document.getElementById('myTextArea').value;
    // var obj = JSON.parse(text);
    var pretty = JSON.stringify(text.value, undefined, 4);
    // document.getElementById('myTextArea').value = pretty;
    return pretty;
  }

  onFileSelected(event) {

    let selectedFile = event.target.id;

    for(let i in this.fileConfigs) {
      if(selectedFile.includes(this.fileConfigs[i].nameTab) &&
         selectedFile.includes(this.fileConfigs[i].nameConfig)){
        console.log('File selected for: ' + this.fileConfigs[i].nameTab + "/" + this.fileConfigs[i].nameConfig);
        this.fileConfigs[i].file = <File>event.target.files[0];
        console.log(this.fileConfigs[i]);
      }
    }

  }


  onUploadFile(nameTab, nameConfig){

    let fileConfigId;
    const fd = new FormData();

    for(let i in this.fileConfigs) {
      if((nameTab === this.fileConfigs[i].nameTab) &&
      (nameConfig === this.fileConfigs[i].nameConfig)){
        fileConfigId = i;
        fd.append(this.fileConfigs[i].nameConfig, this.fileConfigs[i].file, this.fileConfigs[i].location );
        break;
      }
    }

    this._postConfigTabsService.uploadFile(fd)
    .subscribe(
      event => {
        if (event.type === HttpEventType.UploadProgress){
          this.fileConfigs[fileConfigId].uploadProgress = Math.round(event.loaded / event.total * 100);
        } else if (event.type === HttpEventType.Sent ){
            this.toastrService.success("Uploading...",this.fileConfigs[fileConfigId].file.name);
        } else if (event.type === HttpEventType.Response ){
          this.toastrService.success(event.body.message,event.body.data.name);
      }
      },
      error => {
        console.log('Error uploading file', error);
        this.toastrService.danger(error,'Error');
      }
    );

  }

}
