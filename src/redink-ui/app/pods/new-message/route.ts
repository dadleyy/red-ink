import Route from '@ember/routing/route';
import Controller from './controller';
import * as State from './state';

class MessageRoute extends Route {
  public resetController(controller: Controller): void {
    controller.state = State.empty();
  }
}

export default MessageRoute;
