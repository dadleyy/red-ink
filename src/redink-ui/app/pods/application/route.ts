import Route from '@ember/routing/route';
import { inject as service } from '@ember/service';
import EmberIntl from 'ember-intl/services/intl';

class ApplicationRoute extends Route {
  @service
  public declare intl: EmberIntl;

  public beforeModel(): void {
    this.intl.setLocale('en-us');
  }
}

export default ApplicationRoute;
