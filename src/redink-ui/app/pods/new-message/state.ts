import * as Seidr from 'seidr';
import { EmberRunTimer } from '@ember/runloop/types';

export type State = {
  message: string;
  busy: boolean;
  count: Seidr.Maybe<number>;

  poller: Seidr.Maybe<EmberRunTimer>;
  error: Seidr.Maybe<Error>;
};

export function empty(): State {
  return {
    message: '',
    busy: false,
    count: Seidr.Nothing(),
    poller: Seidr.Nothing(),
    error: Seidr.Nothing(),
  };
}

export function setReady(state: State): State {
  return { ...state, busy: false };
}

export function setBusy(state: State): State {
  return { ...state, busy: true, error: Seidr.Nothing() };
}

export function setMessage(state: State, message: string): State {
  return { ...state, message, error: Seidr.Nothing() };
}

export function setCount(state: State, count: number): State {
  return { ...state, count: Seidr.Just(count) };
}

export function setError(state: State, error: Error): State {
  return { ...state, error: Seidr.Just(error) };
}

export function clearError(state: State): State {
  return { ...state, error: Seidr.Nothing() };
}

export function setPoller(state: State, poll: EmberRunTimer): State {
  return { ...state, poller: Seidr.Just(poll) };
}
