import Controller from '@ember/controller';
import config from 'redink-ui/config/environment';
import fetch from 'fetch';
import { action } from '@ember/object';
import { tracked } from '@glimmer/tracking';
import * as State from './state';

class MessageController extends Controller {
  @tracked
  public state?: State.State = State.empty();

  @action
  public async submit(): Promise<void> {
    const { message } = this.state || State.empty();

    if (!message || message.length == 0) {
      return;
    }

    if (message.length > 120) {
      window.alert('Messages must be 120 characters or less');
      return;
    }

    this.state = State.setBusy(this.state || State.empty());

    try {
      const conf = {
        method: 'POST',
        body: JSON.stringify({ message }),
        headers: { 'content-type': 'application/json' },
      };
      await fetch(`${config.apiURL}messages`, { ...conf });
    } catch (error) {
      console.error(error);
    }

    this.state = State.setMessage(this.state, '');
    this.state = State.setReady(this.state);
  }

  @action
  public checked(input: Event): void {
    const code = (input as KeyboardEvent).keyCode;
    if (code === 13) {
      this.submit();
    }
  }

  @action
  public setMessage(input: Event): void {
    const element = (input.target as HTMLInputElement);
    this.state = State.setMessage(this.state || State.empty(), element.value);
  }
}

export default MessageController;
