import { BrowserModule } from '@angular/platform-browser';
import { NgModule } from '@angular/core';
import { FormsModule } from '@angular/forms';   // For Template Driven Forms
import { ReactiveFormsModule } from '@angular/forms';   // For Reactive Forms


import { AppRoutingModule, routingComponents } from './app-routing.module';
import { AppComponent } from './app.component';
// import { ConfigTabsComponent } from './config-tabs/config-tabs.component';
// import { DashboardComponent } from './dashboard/dashboard.component';

import { KeyValueUnsortedPipe } from './key-value-unsorted.pipe';
import { HttpClientModule } from '@angular/common/http';
import { ReplaceUnderscorePipe } from './replace-underscore.pipe';

// Nebular:
import { 
  NbFormFieldModule,
  NbInputModule,
  NbSpinnerModule,
  NbThemeModule,
  NbToastrModule,
  NbSidebarModule,
  NbLayoutModule,
  NbMenuModule,
  NbIconModule,
  NbTabsetModule,
  NbCardModule,
  NbToggleModule,
  NbButtonModule,
  NbToastrConfig,
  NbGlobalPhysicalPosition,
  NbPopoverModule
} from '@nebular/theme';
import { NbEvaIconsModule } from '@nebular/eva-icons';
import { BrowserAnimationsModule } from '@angular/platform-browser/animations';

import { ChartsModule } from 'ng2-charts';


const configToastr: Partial<NbToastrConfig> = {
  position: NbGlobalPhysicalPosition.BOTTOM_LEFT,
  // status: "warning",
  duration: 6*1000    // Duration in ms
};


@NgModule({
  declarations: [
    AppComponent,
    // ConfigTabsComponent,
    // DashboardComponent
    KeyValueUnsortedPipe,
    ReplaceUnderscorePipe,
    routingComponents
  ],
  imports: [
    BrowserModule,
    AppRoutingModule,
    HttpClientModule,
    FormsModule,
    ReactiveFormsModule,
    NbThemeModule.forRoot({ name: 'default' }),
    // NbThemeModule.forRoot({ name: 'dark' }),
    // RouterModule.forRoot(routes, { useHash: true }), // RouterModule.forRoot(routes, { useHash: true }), if this is your app.module
    NbLayoutModule,
    NbSidebarModule.forRoot(), // NbSidebarModule.forRoot(), //if this is your app.module
    NbMenuModule.forRoot(),
    NbIconModule,
    NbTabsetModule,
    NbSpinnerModule,
    NbFormFieldModule,
    NbInputModule,
    // NbToastrModule.forRoot(config),
    // NbToastrModule.forRoot({ position: 'bottom-left' }),
    NbToastrModule.forRoot(configToastr),
    NbPopoverModule,
    BrowserAnimationsModule,
    NbEvaIconsModule,
    NbCardModule,
    ChartsModule,
    NbButtonModule,
    NbToggleModule,
  ],
  providers: [],
  bootstrap: [AppComponent]
})
export class AppModule { }
