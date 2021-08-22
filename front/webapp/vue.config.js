module.exports = {
  transpileDependencies: [
    'vuetify'
  ],

  productionSourceMap: false,

  devServer: {
    proxy: {
      '/api': {
        target: 'http://192.168.1.153:80',
        changeOrigin: true,
        ws: true
      }
    }
  }

}
