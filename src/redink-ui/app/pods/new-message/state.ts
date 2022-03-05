export type State = {
  message: string;
  busy: boolean;
};

export function empty(): State {
  return { message: '', busy: false };
}

export function setReady(state: State): State {
  return { ...state, busy: false };
}

export function setBusy(state: State): State {
  return { ...state, busy: true };
}

export function setMessage(state: State, message: string): State {
  return { ...state, message };
}
