import EmberRouter from '@ember/routing/router';
import config from 'redink-ui/config/environment';

export default class Router extends EmberRouter {
  location = config.locationType;
  rootURL = config.rootURL;
}

Router.map(function () {
  this.route('new-message');
  this.route('missing', { path: '*' });
});
