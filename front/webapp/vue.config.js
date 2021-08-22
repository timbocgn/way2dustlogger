module.exports = {
  transpileDependencies: [
    'vuetify'
  ],

  productionSourceMap: false,

  devServer: {
    proxy: {
      '/api': {
        target: 'http://192.168.0.35:80',
        changeOrigin: true,
        ws: true
      }
    }
  }

}
