import { NgModule } from '@angular/core';
import { Routes, RouterModule } from '@angular/router';

import { ConfigTabsComponent } from './config-tabs/config-tabs.component';
import { DashboardComponent } from './dashboard/dashboard.component';


const routes: Routes = [
  {
    path: 'config-tabs',
    component: ConfigTabsComponent,
  },
  {
    path: 'dashboard',
    component: DashboardComponent,
  },
  { path: '', redirectTo: 'config-tabs', pathMatch: 'full' },
  // { path: '**', redirectTo: 'page-no-found' },

  // {
  //   path: 'pages',
  //   component: ConfigTabsComponent,
  //   // loadChildren: () => import('./pages/pages.module')
  //   //   .then(m => m.PagesModule),
  // }
];

@NgModule({
  imports: [
    // RouterModule.forRoot(routes)
    // RouterModule.forRoot(routes, { useHash: true }), // RouterModule.forRoot(routes, { useHash: true }), if this is your app.module
    RouterModule.forRoot(routes, { useHash: true })
  ],
  exports: [RouterModule]
})
export class AppRoutingModule { }

export const routingComponents = [ConfigTabsComponent, DashboardComponent]
