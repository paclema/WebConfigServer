import { Component, OnInit, OnDestroy, ViewChild } from '@angular/core';
import { webSocket } from "rxjs/webSocket";

import { NbCardModule, NbButtonModule, NbToggleModule, NbFormFieldModule, NbToastrService, NbToastrConfig} from '@nebular/theme';

import { ChartDataSets, ChartOptions, ChartType } from 'chart.js';
import { Color, BaseChartDirective, Label } from 'ng2-charts';
import * as pluginAnnotations from 'chartjs-plugin-annotation';
import { ConfigService } from '../services/config.service';

@Component({
  selector: 'app-dashboard',
  templateUrl: './dashboard.component.html',
  styleUrls: ['./dashboard.component.scss']
})
export class DashboardComponent implements OnInit, OnDestroy {

  subjectWS: any;
  public configData;

  wsObject: any;
  wsValue: any;


  // dashboardData: JSON;
  dashboardData = {};
  labelsHidden: boolean = false;

  public lineChartMaxlength = 100;
  public lineChartData: ChartDataSets[] = [
    // { data: [], label: 'Series A' , yAxisID: 'y-axis-0' },
    // { data: [28, 48, 40, 19, 86, 27, 90], label: 'Series B' },
    // { data: [180, 480, 770, 90, 1000, 270, 400], label: 'Series C', yAxisID: 'y-axis-1' }
  ];
  // public lineChartLabels: Label[] = ['0', '1', '2', '3', '4', '5', '6', '7'];
  public lineChartLabels: Label[] = [];
  public lineChartOptions: (ChartOptions & { annotation: any }) = {
    responsive: true,
    maintainAspectRatio: true,
    scales: {
      // We use this empty structure as a placeholder for dynamic theming.
      // xAxes: [{
          // type: 'time',
          // distribution: 'linear'
          // time: {
          //     unit: 'millisecond'
          // }
      // }],
      xAxes: [{}],
      yAxes: [
        {
          id: 'y-axis-0',
          position: 'left',
        },
        // {
        //   id: 'y-axis-1',
        //   position: 'right',
        //   gridLines: {
        //     color: 'rgba(255,0,0,0.3)',
        //   },
        //   ticks: {
        //     fontColor: 'red',
        //   }
        // }
      ]
    },
    annotation: {
      annotations: [
        {
          type: 'line',
          mode: 'vertical',
          scaleID: 'x-axis-0',
          value: 'March',
          borderColor: 'orange',
          borderWidth: 2,
          label: {
            enabled: true,
            fontColor: 'orange',
            content: 'LineAnno'
          }
        },
      ],
    },
    // https://www.chartjs.org/docs/latest/configuration/animations.html
    animation: {
      duration: 0 // general animation time in ms, 0 to disable animations between updates
    },
    // hover: {
    //     animationDuration: 0 // duration of animations when hovering an item
    // },
    // responsiveAnimationDuration: 0 // animation duration after a resize
  };
  public lineChartColorsBase: Color[] = [
    { // Blue Violet Color Wheel
      backgroundColor: 'rgba(84,13,110,0.3)',
      borderColor: 'rgba(84,13,110,1)',
      pointBackgroundColor: 'rgba(84,13,110,1)',
      pointBorderColor: '#fff',
      pointHoverBackgroundColor: '#fff',
      pointHoverBorderColor: 'rgba(84,13,110,0.8)'
    },
    { // Paradise Pink
      backgroundColor: 'rgba(238,66,102,0.3)',
      borderColor: 'rgba(238,66,102,1)',
      pointBackgroundColor: 'rgba(238,66,102,1)',
      pointBorderColor: '#fff',
      pointHoverBackgroundColor: '#fff',
      pointHoverBorderColor: 'rgba(238,66,102,0.8)'
    },
    { // Sunglow
      backgroundColor: 'rgba(255,210,63,0.3)',
      borderColor: 'rgba(255,210,63,1)',
      pointBackgroundColor: 'rgba(255,210,63,1)',
      pointBorderColor: '#fff',
      pointHoverBackgroundColor: '#fff',
      pointHoverBorderColor: 'rgba(255,210,63,0.8)'
    },
    { // Caribbean Green
      backgroundColor: 'rgba(59,206,172,0.3)',
      borderColor: 'rgba(59,206,172,1)',
      pointBackgroundColor: 'rgba(59,206,172,1)',
      pointBorderColor: '#fff',
      pointHoverBackgroundColor: '#fff',
      pointHoverBorderColor: 'rgba(59,206,172,0.8)'
    },
    { // GO Green
      backgroundColor: 'rgba(14,173,105,0.3)',
      borderColor: 'rgba(14,173,105,1)',
      pointBackgroundColor: 'rgba(14,173,105,1)',
      pointBorderColor: '#fff',
      pointHoverBackgroundColor: '#fff',
      pointHoverBorderColor: 'rgba(14,173,105,0.8)'
    }
    // { // red
    //   backgroundColor: 'rgba(255,0,0,0.3)',
    //   borderColor: 'red',
    //   pointBackgroundColor: 'rgba(148,159,177,1)',
    //   pointBorderColor: '#fff',
    //   pointHoverBackgroundColor: '#fff',
    //   pointHoverBorderColor: 'rgba(148,159,177,0.8)'
    // },
  ];
  public lineChartColors: Color[] = [];
  public lineChartLegend = true;
  public lineChartType: ChartType = 'line';
  public lineChartPlugins = [pluginAnnotations];

  @ViewChild(BaseChartDirective, { static: true }) chart: BaseChartDirective;


  constructor(
    private _configService: ConfigService,
    private toastrService: NbToastrService
    ) {
    for (let i = 0; i < this.lineChartMaxlength+1; i++) {
      this.lineChartLabels.push(i.toString());
    }
    this.lineChartColors[0] = this.lineChartColorsBase[0];

    this.wsObject = "";
    this.wsValue = "";

    this._configService.getConfigData()
    .subscribe(
      data =>{ 
        this.configData = data;
        const iconConfig: Partial<NbToastrConfig> = { icon: 'info', duration: 3000 };


        this.subjectWS = webSocket({
          // url: "ws://"+location.hostname+":81/"
          url: "ws://"+location.hostname+":"+this.configData.services.WebSockets.port,
          openObserver: {
            next: () => {
              console.log('[WS]: connection oppened');
              this.toastrService.info('','[WS]: Connected', iconConfig);
            }
          },
          closeObserver: {
            next: (closeEvent) => {
              console.log('[WS]: connection closed:', closeEvent);
              if(!closeEvent.wasClean) this.toastrService.danger('','[WS]: Disconnected', iconConfig);
            }
          }
        //   deserializer: (msg) => {
        //     // console.log(msg);
        //     // console.log("typeof msg.data");
        //     // console.log(typeof(msg.data));
        //     // return JSON.parse(msg.data)
        //     return msg.data
        //   },
        //   serializer: msg => JSON.stringify(msg)
        
      
        });


        this.subjectWS.subscribe(
          msg => {
            // Called whenever there is a message from the server.
            // console.log('message received: ' + msg);
            // console.log(typeof(msg));
            // if (typeof(msg) == "string") {
            //     this.dashboardData = JSON.parse(msg);
            // }

            // var str;
            // if (typeof(msg) == "string")  console.log("STRING");
            // else if (typeof(msg) == "number") console.log("NUMBER");
            // else if(typeof(msg) == "object") {
            //   str = JSON.stringify(msg);
            //   console.log("OBJECT");
            // }
            // else { 
            //   console.log("FORMAT:");
            //   console.log(typeof(msg));
            // }
            
            if(typeof(msg) == "object") console.log('message received: ' + JSON.stringify(msg));
            else console.log('message received: ' + msg);
            
            // msg contains the whole JSON sent by WS, an example using WebConfigServer basic example:  msg = {"heap_free":76.04,"loop":6,"RSSI":-29}

               this.dashboardData = msg;
   
               // Update Chart line points:
               let index = 0;
               for (let key in this.dashboardData) {
   
                 // If there is a new object received to add a new cahrt line:
                 if (typeof(this.lineChartData[index]) == "undefined"){
                   this.lineChartData.push({ data: [], label: key , yAxisID: 'y-axis-0' } )
                   this.lineChartColors[index] = this.lineChartColorsBase[index%this.lineChartColorsBase.length];
                 }
                 this.lineChartData[index].label = key;
   
                 // Move to the right the list of points
                 if (this.lineChartData[index].data.length >= this.lineChartMaxlength+1)
                   this.lineChartData[index].data = this.lineChartData[index].data.slice(1);
                 this.lineChartData[index].data.push(this.dashboardData[key]);
                 // this.lineChartLabels.push(`Label ${this.lineChartLabels.length}`);
                 index++;
               }
   
   
   
               this.chart.update();
               
   
          },
          err => {  // Called if at any point WebSocket API signals some kind of error.
            console.log('[WS]: error: ',err);
            // this.toastrService.danger('','[WS]: Error', iconConfig);
          },
          () =>{ // Called when connection is closed (for whatever reason).
            console.log('[WS]: subscription finished');
            // this.toastrService.warning('','[WS]: Finished', iconConfig);
          }
        );

      },
      error =>{ 
        this.toastrService.danger(error,'Error');
      }
    );
    

    
  }

  ngOnInit(): void {
  }

  ngOnDestroy(): void {
    this.subjectWS.complete(); // Closes the connection.
  }


  sendMessage() {
    // this.webSocket.send({ message: this.msgCtrl.value });
    // this.msgCtrl.setValue('');
    // this.subjectWS.next(JSON.parse('{"message": "dashboard sent some message"}'));

    // this.subjectWS.next({message: 'dashboard sent some message'});
    // This will send a message to the server once a connection is made. Remember value is serialized with JSON.stringify by default!
    
    if( this.wsObject == "") 
      this.subjectWS.next({ "message": this.wsValue});
    else 
      this.subjectWS.next({ [this.wsObject] : this.wsValue});
    
  }

  sendMessageBroadcast() {
    // this.webSocket.send({ message: this.msgCtrl.value });
    // this.msgCtrl.setValue('');
    // this.subjectWS.next(JSON.parse('{"broadcast": "hi everyone"}'));
    // this.subjectWS.next({broadcast: 'hi everyone'});
    // This will send a message to the server once a connection is made. Remember value is serialized with JSON.stringify by default!
    
    this.subjectWS.next({broadcast: this.wsValue});

  }

  // This function its added to use keyvalue pipe under *ngFor to get the
  // configTabsForm.controls unsorted:
  returnZero() {
    return 0
  }

  // Chart events:
  public chartClicked({ event, active }: { event: MouseEvent, active: {}[] }): void {
    console.log(event, active);
  }

  public chartHovered({ event, active }: { event: MouseEvent, active: {}[] }): void {
    console.log(event, active);
  }

  public hideAll(): void {
    this.labelsHidden= !this.labelsHidden;
    for (let index = 0; index < this.lineChartData.length; index++) {
      this.chart.hideDataset(index, this.labelsHidden);
    }
  }


}
