import Route from '@ember/routing/route';
import Router from '@ember/routing/router';
import { inject as service } from '@ember/service';

class MissingRoute extends Route {
  @service
  public declare router: Router;

  public beforeModel(): void {
    const { router } = this;
    router.transitionTo('new-message');
  }
}

export default MissingRoute;
