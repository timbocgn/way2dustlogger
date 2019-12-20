module.exports = {

  productionSourceMap: false,

  devServer: {
    proxy: {
      '/api': {
        target: 'http://192.168.1.67:80',
        changeOrigin: true,
        ws: true
      }
    }
  }
}
