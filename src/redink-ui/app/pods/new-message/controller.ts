import Controller from '@ember/controller';
import config from 'redink-ui/config/environment';
import fetch from 'fetch';
import { action } from '@ember/object';
import { later, cancel } from '@ember/runloop';
import { tracked } from '@glimmer/tracking';
import debugLogger from 'ember-debug-logger';
import * as State from 'redink-ui/pods/new-message/state';

const debug = debugLogger('controller:new-message');

class MessageController extends Controller {
  @tracked
  public state: State.State = State.empty();

  @action
  public async submit(): Promise<void> {
    const { message } = this.state;

    if (!message || message.length == 0) {
      return;
    }

    if (message.length > 120) {
      window.alert('Messages must be 120 characters or less');
      return;
    }

    this.state.poller.map(cancel);
    this.state = State.setBusy(this.state);

    try {
      const conf = {
        method: 'POST',
        body: JSON.stringify({ message }),
        headers: { 'content-type': 'application/json' },
      };
      await fetch(`${config.apiURL}messages`, { ...conf });
    } catch (error) {
      this.state = State.setReady(State.setError(this.state, error));
      return;
    }

    const cleared = State.setPoller(
      State.setMessage(this.state, ''),
      later(() => this.poll(), 1000)
    );
    this.state = State.setReady(cleared);
  }

  @action
  public checked(input: Event): void {
    const code = (input as KeyboardEvent).keyCode;

    if (code === 13) {
      input.stopPropagation();
      input.preventDefault();
      this.submit();
    }
  }

  @action
  public setMessage(input: Event): void {
    const { state } = this;
    const element = input.target as HTMLInputElement;
    const next = State.setMessage(state, element.value);
    next.poller.map(cancel);
    this.state = State.setPoller(
      next,
      later(() => this.poll(), 1000)
    );
  }

  @action
  public async poll(): Promise<void> {
    const { state } = this;
    debug('performing poll');

    try {
      const conf = { method: 'GET' };
      const response = await fetch(`${config.apiURL}status`, { ...conf });
      const { size: count } = await response.json();
      debug('poll success, count "%s"', count);
      this.state = State.setCount(this.state, count);
    } catch (error) {
      this.state = State.setError(state, error);
      return;
    }

    debug('preparing for next poll');
    this.state = State.setPoller(
      this.state,
      later(() => this.poll(), 1000)
    );
  }

  @action
  public cleanup(): void {
    const { state } = this;
    debug('performing cleanup');

    if (state) {
      state.poller.map(cancel);
    }
  }

  @action
  public clearError(): void {
    const { state } = this;
    const next = State.clearError(state);
    next.poller.map(cancel);
    this.state = State.setPoller(
      next,
      later(() => this.poll(), 1000)
    );
  }
}

export default MessageController;
