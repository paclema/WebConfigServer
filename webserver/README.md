# Webserver

To update/install the local node_modules run `npm install`

This project was generated with [Angular CLI](https://github.com/angular/angular-cli) version 10.1.7.

## Development server

Run `ng serve --configuration=dev` for a dev server. Navigate to `http://localhost:4200/`. The app will automatically reload if you change any of the source files.

## Code scaffolding

Run `ng generate component component-name` to generate a new component. You can also use `ng generate directive|pipe|service|class|guard|interface|enum|module`.

## Build

Run `ng build` to build the project. The build artifacts will be stored in the `dist/` directory. Use the `--prod` flag for a production build.

Run `ng build --watch --configuration=dev` to generate also build code in /webserver/dist folder and have webserver/src/config/config.json accessible.
ng serve  --configuration=dev

## Running unit tests

Run `ng test` to execute the unit tests via [Karma](https://karma-runner.github.io).

## Running end-to-end tests

Run `ng e2e` to execute the end-to-end tests via [Protractor](http://www.protractortest.org/).

## To add bootsrap into the webserver:
Follow this [tutorial](https://therichpost.com/angular-10-bootstrap-4-tab-nav-pills-working-example/) and install bootstrap and jquery node modules:

```console
npm i jquery --save
npm i bootstrap --save
```

And add the styles and scripts to angular.json:

```json
"styles": [
             "node_modules/bootstrap/dist/css/bootstrap.min.css",

           ],
           "scripts": [
             "node_modules/jquery/dist/jquery.js",
             "node_modules/bootstrap/dist/js/bootstrap.js",
```

## Further help

To get more help on the Angular CLI use `ng help` or go check out the [Angular CLI README](https://github.com/angular/angular-cli/blob/master/README.md).
