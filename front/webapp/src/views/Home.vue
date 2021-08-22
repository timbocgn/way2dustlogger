<template>
  <v-container>
    <v-simple-table>
    
          <thead>
            <tr>
              <th colspan=3 class="text-left">Sensor Data</th>
            </tr>
          </thead>

          <tbody>   
            <template v-if="values.length != 0">
              <div v-for="(item,index) in values" :key="index">

                  <tr>
                    <td>Sensor {{index+1}}</td>
                    <td class="grey--text">Value</td> 
                    <td class="grey--text">Unit</td> 
                  </tr>

                  <template v-if="loaded[index]==true">
                      <tr>
                        <td class="grey--text">Temperature</td> 
                        <td>{{item.temp.toFixed(2)}}</td>
                        <td class="grey--text"> °C</td> 
                      </tr>

                      <tr>
                        <td class="grey--text">Relative Humidity</td> 
                        <td>{{item.rh.toFixed(2)}}</td>
                        <td class="grey--text"> %</td> 
                      </tr>

                      <tr>
                        <td class="grey--text">Dew Point</td> 
                        <td>{{item.dp.toFixed(2)}}</td>
                        <td class="grey--text"> °C</td> 
                      </tr>                  
                  </template>
                  <template v-else>
                      <tr>
                        <td colspan=3 class="grey--text">Loading...</td> 
                      </tr>
                </template>
              </div>
            </template>
            <template v-else>
              <tr>
                <td colspan=3 class="grey--text">Loading...</td> 
              </tr>
            </template>
          </tbody>
      </v-simple-table> 
  </v-container>
</template>



<script>
export default {
  data() {
    return {
      sensorcnt: null,
      values: [],
      loaded: [],
      timer: null
    };
  },
  
  // ---- cleanup the timer object

  destroyed()
  {
    clearInterval(this.timer);
  },

  // ---- define some custom methods

  methods: 
  {
    // ---- this one calls the AJAX functions and updates 

    updateData: function() 
    {
          this.$ajax
          .get("/api/v1/sensorcnt")
          .then(data => {

            this.sensorcnt  = data.data.cnt;

            var i;
            for (i = 0; i < this.sensorcnt; i++) 
            { 
                //console.log("Send request %d",i)
                var urlidx = i+1;
    
                this.$ajax
                          .get("/api/v1/temp/" + urlidx.toString(),{ id: i})
                          .then(data => {
                                          this.values[data.config.id] = {temp: data.data.temp, rh: data.data.rh, dp: data.data.dp};
                                          this.loaded[data.config.id] = true;
                                          this.$forceUpdate();

                                          //console.log("Got Values %d",data.config.id);

                                          //console.log(data);
                                      }
                                )
                          .catch(error => {
                                          console.log(error);
                                        }
                                );
            }
          })
          .catch(error => {
            console.log(error);
          });    
    }
  }, 

  // ---- setup the timer

  mounted() 
  {
      clearInterval(this.timer);
      this.timer = setInterval(this.updateData , 1000);
  }
};
</script>
