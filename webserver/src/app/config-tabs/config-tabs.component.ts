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


@Component({
  selector: 'app-config-tabs',
  templateUrl: './config-tabs.component.html',
  styleUrls: ['./config-tabs.component.scss']
})
export class ConfigTabsComponent implements OnInit {

  public configData;
  public errorMsg;


  // For the Template Driven Form:
  public errorMsgPost = false;
  public dataMsgPost;

  public enableTutorial = false;

  testFormModel = {
    "userName": "Pablo",
    "email": "email@gmail.com",
    "phone": "666666666",
    "sleepMode": "Keep awake",
    "timePreference": "evening",
    "ota": "true",

  };

  sleepModes = ['Deep sleep', 'Light sleep', 'Keep awake'];
  sleepModeHasError = false;

  submitted = false;

  // For the (manual) Reactive Form:
  // registrationForm = new FormGroup({
  //   userName: new FormControl('paclema'),
  //   password: new FormControl(''),
  //   confirmPassword: new FormControl(''),
  //   address: new FormGroup({
  //     city: new FormControl('Coslada'),
  //     state: new FormControl('Madrid'),
  //     postalCode: new FormControl('28820')
  //   })
  // });

  // For the (auto) Reactive Form:
  // Moved to OnInit() to add conditional validation and subscribe for the observable
  /*
  registrationForm = this.fb.group({
    userName: ['', [Validators.required, Validators.minLength(3), ForbiddenNameValidator(/admin/)]],
    email: [''],
    subscribe: [false],
    password: [''],
    confirmPassword: [''],
    address: this.fb.group({
      city: [''],
      state: [''],
      postalCode: ['']
    })
  }, {validator: PasswordValidator});
  */
  // And add the FormGroup property:
  registrationForm: FormGroup;
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
    // this.loadApiData();

    // For the (auto) Reactive Form:
    this.registrationForm = this.fb.group({
      userName: ['', [Validators.required, Validators.minLength(3), ForbiddenNameValidator(/admin/)]],
      email: [''],
      subscribe: [false],
      password: [''],
      confirmPassword: [''],
      address: this.fb.group({
        city: [''],
        state: [''],
        postalCode: ['']
      }),
      alternateEmails: this.fb.array([])    // Dynamic FormArray initialy empty
    }, {validator: PasswordValidator});

    // Subscribe to the Observable to receive "subscribe" FormControl value changes:
    this.registrationForm.get('subscribe').valueChanges
          .subscribe(checkedValue => {
            const email = this.registrationForm.get('email');
            if (checkedValue){
              email.setValidators(Validators.required);
            } else {
              email.clearValidators();
            }
            email.updateValueAndValidity();
          });


          
  }

  // For the Template Driven Form:
  validateSleepMode(value){
    if((value === "default" ) || (value === ''))
      this.sleepModeHasError = true;
    else
      this.sleepModeHasError = false;
  }

  onSubmit(testForm){

    // The property of this class:
    console.log(this.testFormModel);
    // this.errorMsgPost = false;

    // The whole ngFormGroup status and data, received onSubmit():
    console.log(testForm);

    this._enrollmentService.enroll(this.testFormModel)
      .subscribe(
        data => {console.log('Success posting the data', data);
                  this.dataMsgPost = data;
                  this.submitted = true;},
        error => {console.log('Error posting the data', error);
                  this.errorMsgPost = error;
                  this.submitted = false;}
      )

  }


  // For the Reactive Form:
  loadApiData(){
    // To set each FormControl values for the Reactive FormGrop:
    // setValue() accepts an object that matches the structure of the FormGroup
    // the object must contain all keys

    // this.registrationForm.setValue({
    //   userName: 'name',
    //   password: 'password',
    //   confirmPassword: 'password',
    //   address: {
    //     city: 'City',
    //     state:'State',
    //     postalCode: '123456'
    //     }
    //   });

    // If only a few keys want to be set use patchValue():
    this.registrationForm.patchValue({
      userName: 'name',
      // password: 'password',
      // confirmPassword: 'password',
      address: {
        city: 'City',
        state:'State',
        // postalCode: '123456'
        }
      });
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

  get userName(){
    return this.registrationForm.get('userName');
  }

  get email(){
    return this.registrationForm.get('email');
  }

  get alternateEmails(){
    return this.registrationForm.get('alternateEmails') as FormArray;
  }

  addAlternativeEmails(){
    this.alternateEmails.push(this.fb.control(''));
  }

  onSubmitRF(){
    console.log(this.registrationForm.value);

    this._postConfigTabsService.register(this.registrationForm.value)
    .subscribe(
      response => {console.log('Success posting the data', response);
                this.dataMsgPost = response;
                this.submitted = true;},
      error => {console.log('Error posting the data', error);
                this.errorMsgPost = error;
                this.submitted = false;}
    )

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

      }
      // console.log('newTabForm: ');
      // console.log(newTabForm);
      // newTabForm.setParent(this.configTabsForm);
      this.configTabsForm.addControl(tab, newTabForm);

    }
    console.log('Form built: configTabsForm');
    console.log(this.configTabsForm);
    // console.log('Form built: configTabsForm.value');
    console.log(this.configTabsForm.value);
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

  typeConfig(obj): string {
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

  log(val) { console.log(val); }

  prettyPrint(text) {
    // var ugly = document.getElementById('myTextArea').value;
    // var obj = JSON.parse(text);
    var pretty = JSON.stringify(text.value, undefined, 4);
    // document.getElementById('myTextArea').value = pretty;
    return pretty;
  }

}
