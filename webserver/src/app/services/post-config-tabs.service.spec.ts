import { TestBed } from '@angular/core/testing';

import { PostConfigTabsService } from './post-config-tabs.service';

describe('PostConfigTabsService', () => {
  let service: PostConfigTabsService;

  beforeEach(() => {
    TestBed.configureTestingModule({});
    service = TestBed.inject(PostConfigTabsService);
  });

  it('should be created', () => {
    expect(service).toBeTruthy();
  });
});
