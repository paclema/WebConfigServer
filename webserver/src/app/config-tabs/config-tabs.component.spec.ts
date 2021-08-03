import { ComponentFixture, TestBed } from '@angular/core/testing';

import { ConfigTabsComponent } from './config-tabs.component';

describe('ConfigTabsComponent', () => {
  let component: ConfigTabsComponent;
  let fixture: ComponentFixture<ConfigTabsComponent>;

  beforeEach(async () => {
    await TestBed.configureTestingModule({
      declarations: [ ConfigTabsComponent ]
    })
    .compileComponents();
  });

  beforeEach(() => {
    fixture = TestBed.createComponent(ConfigTabsComponent);
    component = fixture.componentInstance;
    fixture.detectChanges();
  });

  it('should create', () => {
    expect(component).toBeTruthy();
  });
});
