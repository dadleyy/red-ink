'use strict';

/* eslint-disable @typescript-eslint/no-var-requires */
const EmberApp = require('ember-cli/lib/broccoli/ember-app');
const autoprefixer = require('autoprefixer');
const tailwindcss = require('tailwindcss');
/* eslint-enable @typescript-eslint/no-var-requires */

module.exports = function (defaults) {
  let app = new EmberApp(defaults, {
    postcssOptions: {
      compile: {
        enable: true,
        cacheInclude: [/.*\.(css|scss|sass|less|hbs)$/],
        plugins: [
          {
            module: autoprefixer,
          },
          {
            module: tailwindcss,
            options: require('./tailwind.config.js'),
          },
        ],
      },
    },
  });

  return app.toTree();
};
