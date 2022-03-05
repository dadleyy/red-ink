use std::io::{Error, ErrorKind, Result};

use serde::Deserialize;

#[derive(Deserialize, Debug, Clone)]
struct MessagePayload {
  message: String,
}

#[derive(Debug, Default, Clone)]
struct RedisConfig {
  addr: String,
  queue: String,
}

#[derive(Debug, Default, Clone)]
struct State {
  redis: RedisConfig,
}

async fn push(mut request: tide::Request<State>) -> tide::Result {
  let payload = match request.body_json::<MessagePayload>().await {
    Err(error) => {
      log::warn!("unable to parse json - {error}");
      return Ok(tide::Response::builder(422).build());
    }
    Ok(payload) => payload,
  };

  log::info!("pushing new message - '{}'", payload.message);
  let state = request.state();
  let mut client = match async_std::net::TcpStream::connect(&state.redis.addr).await {
    Err(error) => {
      log::warn!("unable to connect to redis - {error}");
      return Ok(tide::Response::builder(500).build());
    }
    Ok(client) => client,
  };
  match kramer::execute(
    &mut client,
    kramer::Command::List(kramer::ListCommand::Push(
      (kramer::Side::Left, kramer::Insertion::Always),
      &state.redis.queue,
      kramer::Arity::One(&payload.message),
    )),
  )
  .await
  {
    Err(error) => {
      log::warn!("unable to push message - {error}");
      return Ok(tide::Response::builder(500).build());
    }
    Ok(result) => log::info!("successfully sent - {:?}", result),
  }

  Ok("ok".into())
}

async fn missing(request: tide::Request<State>) -> tide::Result {
  log::debug!("route not found - {:?}", request.url());

  Ok(
    tide::Response::builder(404)
      .body(tide::Body::from_string("not-found".into()))
      .build(),
  )
}

async fn run<S>(mut app: tide::Server<State>, addr: S) -> Result<()>
where
  S: std::fmt::Display,
{
  app.at("/").all(missing);
  app.at("/*").all(missing);

  app.at("/messages").post(push);

  log::info!("main web thread running");
  app.listen(format!("{}", addr)).await
}

fn main() -> Result<()> {
  dotenv::dotenv().map_err(|error| Error::new(ErrorKind::Other, error))?;
  env_logger::init();

  log::info!("booting redink server");

  let addr = std::env::var("REDINK_ADDR").unwrap_or("0.0.0.0:8081".into());

  log::info!("preparing server on '{}'", addr);
  let app = tide::with_state(State {
    redis: RedisConfig {
      addr: std::env::var("REDIS_ADDR").map_err(|error| {
        log::warn!("unable to find 'REDIS_ADDR' in env - {error}");
        Error::new(ErrorKind::Other, format!("{error}"))
      })?,
      queue: std::env::var("REDIS_MESSAGE_QUEUE").map_err(|error| {
        log::warn!("unable to find 'REDIS_MESSAGE_QUEUE' in env - {error}");
        Error::new(ErrorKind::Other, format!("{error}"))
      })?,
    },
  });

  async_std::task::block_on(run(app, &addr))
}
